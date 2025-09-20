#pragma once
#ifndef WSOCKET__ASIO_KEEPALIVE_MANAGER_HPP
#define WSOCKET__ASIO_KEEPALIVE_MANAGER_HPP

#ifdef WITH_ASIO

#include <asio.hpp>

namespace wsocket {

class KeepAliveManager {
public:
    explicit KeepAliveManager(asio::any_io_executor io_executor) :
        expired_timer_(io_executor), timeout_timer_(io_executor), expired_time_ms_(EXPIRED_TIME_MS_DEFAULT),
        timeout_ms_(TIMEOUT_MS_DEFAULT) {}

    ~KeepAliveManager() { this->Stop(); }

    static constexpr int64_t EXPIRED_TIME_S_DEFAULT  = 2 * 60; // 2min
    static constexpr int64_t EXPIRED_TIME_MS_DEFAULT = EXPIRED_TIME_S_DEFAULT * 1000;
    static constexpr int64_t TIMEOUT_MS_DEFAULT      = 3 * EXPIRED_TIME_MS_DEFAULT;

    // 设置过期时间（秒）
    void SetExpiredTimeSec(int64_t sec) {
        expired_time_ms_ = std::chrono::milliseconds(sec * 1000);
        timeout_ms_      = 3 * expired_time_ms_;
    }

    // 设置过期时间（毫秒）
    void SetExpiredTimeMsec(int64_t msec) {
        expired_time_ms_ = std::chrono::milliseconds(msec);
        timeout_ms_      = 3 * expired_time_ms_;
    }

    // 启动或刷新定时器
    void Start() { Flush(); }

    // 刷新定时器，重新开始计时
    void Flush() {
        // 取消之前的定时器
        expired_timer_.cancel();
        timeout_timer_.cancel();

        // 设置新的过期定时器
        expired_timer_.expires_after(expired_time_ms_);
        expired_timer_.async_wait([this](std::error_code ec) { OnExpiredTimerTimeout(ec); });

        // 设置新的超时定时器
        timeout_timer_.expires_after(timeout_ms_);
        timeout_timer_.async_wait([this](std::error_code ec) { OnTimeoutTimerTimeout(ec); });
    }

    void Stop() {
        expired_timer_.cancel();
        timeout_timer_.cancel();
    }

    class Listener {
    public:
        virtual ~Listener()                                 = default;
        virtual void OnKeepAliveExpired(std::error_code ec) = 0;
        virtual void OnKeepAliveTimeout(std::error_code ec) = 0;
    };
    void ResetListener(Listener *listener) { listener_ = listener; }

private:
    // 过期定时器超时处理
    void OnExpiredTimerTimeout(std::error_code ec) {
        if(ec == asio::error::operation_aborted) {
            // 定时器被取消，不需要处理
            return;
        }

        // 通知监听器保活已过期
        if(listener_) {
            listener_->OnKeepAliveExpired(ec);
        }
    }

    // 超时定时器超时处理
    void OnTimeoutTimerTimeout(std::error_code ec) {
        if(ec == asio::error::operation_aborted) {
            // 定时器被取消，不需要处理
            return;
        }
        // 通知监听器已超时
        if(listener_) {
            listener_->OnKeepAliveTimeout(ec);
        }
    }


private:
    asio::steady_timer        expired_timer_;   // 保活过期定时器
    asio::steady_timer        timeout_timer_;   // 连接超时定时器
    std::chrono::milliseconds expired_time_ms_; // 保活过期时间
    std::chrono::milliseconds timeout_ms_;      // 连接超时时间
    Listener                 *listener_{nullptr};
};

} // namespace wsocket

#endif

#endif // WSOCKET__ASIO_KEEPALIVEMANAGER_HPP
