#pragma once
#ifndef WSOCKET__ASIO_WSOCKET_HPP
#define WSOCKET__ASIO_WSOCKET_HPP

#ifdef WITH_ASIO

#include <asio.hpp>

#include "WSocketContext.hpp"
#include "ASIO_KeepAliveManager.hpp"

namespace wsocket {

template <typename Protocol>
class WSocketBase : public WSocketContext::Listener,
                    public KeepAliveManager::Listener,
                    public std::enable_shared_from_this<WSocketBase<Protocol>> {

    using socket_type   = typename Protocol::socket;
    using endpoint_type = typename Protocol::endpoint;

protected:
    explicit WSocketBase(asio::any_io_executor io_executor) :
        socket_(asio::make_strand(io_executor)), keep_alive_manager_(socket_.get_executor()) {
        Initialize();
    }
    explicit WSocketBase(socket_type &&socket) :
        socket_(std::move(socket)), keep_alive_manager_(socket_.get_executor()) {
        Initialize();
    }

private:
    // Initialize common settings
    void Initialize() {
        keep_alive_manager_.ResetListener(this);
        wsocket_context_.AddListener(this);

        // Set send handler
        wsocket_context_.ResetSendHandler([this](Buffer buffer) {
            asio::error_code ec;
            this->socket_.send(asio::buffer(buffer.buf, buffer.size), 0, ec);
            if(ec) {
                this->OnError(ec);
            }
        });
    }

public:
    static std::shared_ptr<WSocketBase> Create(asio::any_io_executor io_executor) {
        return std::shared_ptr<WSocketBase>(new WSocketBase(std::move(io_executor)));
    }
    static std::shared_ptr<WSocketBase> Create(socket_type &&socket) {
        return std::shared_ptr<WSocketBase>(new WSocketBase(std::move(socket)));
    }

    ~WSocketBase() override {
        wsocket_context_.DelListener(this);
        wsocket_context_.ResetSendHandler(nullptr);
        keep_alive_manager_.ResetListener(nullptr);

        if(socket_.is_open()) {
            asio::error_code ec;
            std::ignore = socket_.close(ec); // Ignore close errors
        }
    }

    void Start() {
        this->StartRecv();
        keep_alive_manager_.Start();
    }

    // Set keep-alive expiration time
    void SetKeepAliveExpiredTime(int64_t expire_ms) {
        keep_alive_manager_.SetExpiredTimeMsec(expire_ms);
        keep_alive_manager_.Flush();
    }

    // Initiate connection and perform WSocket handshake
    void Handshake(const endpoint_type &endpoint) {
        auto _this = this->shared_from_this();
        socket_.async_connect(endpoint, [=](asio::error_code ec) {
            if(ec) {
                this->OnError(ec);
                return;
            }
            _this->OnSocketConnected();
        });
    }

    // Send Ping frame
    void Ping() { this->wsocket_context_.Ping(); }

    // Send Pong frame
    void Pong() { this->wsocket_context_.Pong(); }

    // Send text message
    void Text(std::string_view text, bool finish = true) { this->wsocket_context_.SendText(text, finish); }

    // Send binary message
    void Binary(Buffer buffer, bool finish = true) { this->wsocket_context_.SendBinary(buffer, finish); }

    // Close connection (using standard close code)
    void Close(CloseCode code) { this->wsocket_context_.Close(code); }

    // Close connection (using custom close code and reason)
    void Close(int16_t code, const std::string &reason) { this->wsocket_context_.Close(code, reason); }

protected:
    //============ WSocketContext::Listener start ============//
    void OnError(std::error_code code) override {}
    void OnConnected() override {}
    void OnClose(int16_t code, const std::string &reason) override {}
    void OnPing() override { this->wsocket_context_.Pong(); }
    void OnPong() override {}
    void OnText(std::string_view text, bool finish) override {}
    void OnBinary(Buffer buffer, bool finish) override {}
    //============ WSocketContext::Listener end ============//

    //============ KeepAliveManager::Listener start ============//
    void OnKeepAliveExpired(std::error_code ec) override {
        if(ec == asio::error::operation_aborted) {
            return;
        }
        this->keep_alive_manager_.Flush();
        this->wsocket_context_.Ping();
    }
    void OnKeepAliveTimeout(std::error_code ec) override {
        if(ec == asio::error::operation_aborted) {
            return;
        }
        this->wsocket_context_.Close(CloseCode::CLOSE_PROTOCOL_ERROR);

        asio::error_code ignore_ec;
        std::ignore = socket_.shutdown(socket_type::shutdown_both, ignore_ec);

        this->OnError(Error::KeepAliveTimeout);
    }
    //============ KeepAliveManager::Listener end ============//

private:
    // connection success callback
    void OnSocketConnected() {
        this->Start();
        this->wsocket_context_.Handshake();
    }
    // Data receive completion callback
    void OnReceived(std::size_t bytes_transferred) {
        this->wsocket_context_.CommitWrite(bytes_transferred);
        this->StartRecv();
    }

    // Start asynchronous data reception
    void StartRecv() {
        auto _this = this->shared_from_this();
        auto buf   = wsocket_context_.PrepareWrite();
        socket_.async_receive(asio::buffer(buf.buf, buf.size), [=](std::error_code ec, std::size_t bytes_transferred) {
            if(ec) {
                _this->OnError(ec);
                return;
            }
            _this->OnReceived(bytes_transferred);
        });
    }

private:
    socket_type      socket_;
    KeepAliveManager keep_alive_manager_;
    WSocketContext   wsocket_context_;
};

using WSocket = WSocketBase<asio::ip::tcp>;
#ifdef ASIO_HAS_LOCAL_SOCKETS
using UnixWSocket = WSocketBase<asio::local::stream_protocol>;
#endif


} // namespace wsocket

#endif

#endif // WSOCKET__ASIO_WSOCKET_HPP
