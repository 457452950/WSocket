#include <bitset>
#include <cassert>
#include <iostream>

#include "include/WSocketContext.hpp"
#include "include/ASIO_WSocket.hpp"

void testBasicHeader() {
    wsocket::BasicHeader header;
    header.fin            = (true);
    header.rsv1           = (true);
    header.rsv2           = (true);
    header.rsv3           = (true);
    header.opcode         = (0b0101);
    header.payload_length = 124;

    assert(header.fin == true);
    assert(header.rsv1 == true);
    assert(header.rsv2 == true);
    assert(header.rsv3 == true);
    assert(header.opcode == 0b0101);
    assert(header.payload_length == 124);

    uint8_t value[2] = {0};
    memcpy(&value, &header, sizeof(header));
    std::cout << std::bitset<8>(value[0]) << std::bitset<8>(value[1]) << std::endl;
}

void testFrameHeader() {
    {
        wsocket::FrameHeader header;

        uint8_t value[8] = {};
        value[0]         = 0b1100'1000;
        value[1]         = 127;

        memcpy(&header, &value, sizeof(header));
        assert(header.Finished() == true);
        assert(header.Compressed() == true);
        assert(header.Type() == wsocket::FrameHeader::Close);
        assert(header.Length() == 127);
    }
    {
        wsocket::FrameHeader header;

        uint8_t value[10] = {};
        value[0]          = 0b1100'1000;
        value[1]          = 0b1111'1110;
        value[2]          = 0b0000'0000;
        value[3]          = 0b1111'1111;

        value[4] = 0b1111'1111;
        value[5] = 0b1111'1111;
        value[6] = 0b1111'1111;
        value[7] = 0b1111'1111;
        value[8] = 0b1111'1111;
        value[9] = 0b1111'1111;

        memcpy(&header, &value, sizeof(header));
        assert(header.Finished() == true);
        assert(header.Compressed() == true);
        assert(header.Type() == wsocket::FrameHeader::Close);
        std::cout << "length : " << header.Length() << " " << std::bitset<16>(header.Length()) << std::endl;
        assert(header.Length() == 255);
    }
    {
        wsocket::FrameHeader header;

        uint8_t value[10] = {};
        value[0]          = 0b1100'1000;
        value[1]          = 0b1111'1110;
        value[2]          = 0b0000'0100;
        value[3]          = 0b1010'0011;

        value[4] = 0b1010'0101;
        value[5] = 0b1010'0101;
        value[6] = 0b1010'0101;
        value[7] = 0b1010'0101;
        value[8] = 0b1010'0101;
        value[9] = 0b1010'0101;

        memcpy(&header, &value, sizeof(header));
        assert(header.Finished() == true);
        assert(header.Compressed() == true);
        assert(header.Type() == wsocket::FrameHeader::Close);
        std::cout << "length : " << header.Length() << " " << std::bitset<16>(header.Length()) << std::endl;
        assert(header.Length() == 0b0000'0100'1010'0011);
    }
    {
        wsocket::FrameHeader header;

        uint8_t value[10] = {};
        value[0]          = 0b1100'1000;
        value[1]          = 0b1111'1111;

        value[2] = 0b0000'0000;
        value[3] = 0b1111'1111;
        value[4] = 0b1111'1111;
        value[5] = 0b1111'1111;
        value[6] = 0b1111'1111;
        value[7] = 0b1111'1111;
        value[8] = 0b1111'1111;
        value[9] = 0b1111'1111;

        memcpy(&header, &value, sizeof(header));
        assert(header.Finished() == true);
        assert(header.Compressed() == true);
        assert(header.Type() == wsocket::FrameHeader::Close);
        std::cout << "length : " << header.Length() << " " << std::bitset<64>(header.Length()) << std::endl;
        assert(header.Length() == 0x00ff'ffff'ffff'ffff);
    }
    {
        wsocket::FrameHeader header;
        header.Finished(true);
        header.Compressed(true);
        header.Type(wsocket::FrameHeader::Pong);
        header.Length(127);

        uint8_t value[10] = {};
        memcpy(&value[0], &header, sizeof(header));

        assert(value[0] == 0b1100'1010);
        assert(value[1] == 0b0111'1111);
    }
    {
        wsocket::FrameHeader header;
        header.Finished(true);
        header.Compressed(true);
        header.Type(wsocket::FrameHeader::Pong);
        header.Length(256);

        uint8_t value[10] = {};
        memcpy(&value[0], &header, sizeof(header));

        assert(value[0] == 0b1100'1010);
        assert(value[1] == 0b1111'1110);

        assert(value[2] == 0b0000'0001);
        assert(value[3] == 0b0000'0000);
    }
    {
        wsocket::FrameHeader header;
        header.Finished(true);
        header.Compressed(true);
        header.Type(wsocket::FrameHeader::Pong);
        header.Length(264);

        uint8_t value[10] = {};
        memcpy(&value[0], &header, sizeof(header));

        assert(value[0] == 0b1100'1010);
        assert(value[1] == 0b1111'1110);

        assert(value[2] == 0b0000'0001);
        assert(value[3] == 0b0000'1000);
    }
    {
        wsocket::FrameHeader header;
        header.Finished(true);
        header.Compressed(true);
        header.Type(wsocket::FrameHeader::Pong);
        header.Length(264);

        uint8_t value[10] = {};
        memcpy(&value[0], &header, sizeof(header));

        assert(value[0] == 0b1100'1010);
        assert(value[1] == 0b1111'1110);

        assert(value[2] == 0b0000'0001);
        assert(value[3] == 0b0000'1000);
    }
    {
        wsocket::FrameHeader header;
        header.Finished(true);
        header.Compressed(true);
        header.Type(wsocket::FrameHeader::Pong);
        header.Length(std::numeric_limits<uint64_t>::max());

        uint8_t value[10] = {};
        memcpy(&value[0], &header, sizeof(header));

        assert(value[0] == 0b1100'1010);
        assert(value[1] == 0b1111'1111);

        assert(value[2] == 0b1111'1111);
        assert(value[3] == 0b1111'1111);
        assert(value[4] == 0b1111'1111);
        assert(value[5] == 0b1111'1111);
        assert(value[6] == 0b1111'1111);
        assert(value[7] == 0b1111'1111);
        assert(value[8] == 0b1111'1111);
        assert(value[9] == 0b1111'1111);
    }
    {
        wsocket::FrameHeader header;
        header.Finished(true);
        header.Compressed(true);
        header.Type(wsocket::FrameHeader::Pong);
        header.Length(55169595);

        uint8_t value[10] = {};
        memcpy(&value[0], &header, sizeof(header));

        assert(value[0] == 0b1100'1010);
        assert(value[1] == 0b1111'1111);

        assert(value[2] == 0b0000'0000);
        assert(value[3] == 0b0000'0000);
        assert(value[4] == 0b0000'0000);
        assert(value[5] == 0b0000'0000);
        assert(value[6] == 0b0000'0011);
        assert(value[7] == 0b0100'1001);
        assert(value[8] == 0b1101'0010);
        assert(value[9] == 0b0011'1011);

        assert(header.Length() == 55169595);
    }
}

std::string to_string(const wsocket::Buffer &buffer) {
    return std::string(reinterpret_cast<char *>(buffer.buf), buffer.size);
}

void test_SlidingBuffer() {
    {
        wsocket::SlidingBuffer buffer;
        char                   data[] = "abcdefghijklmnopqrst";
        buffer.Feed({reinterpret_cast<uint8_t *>(data), 20});
        auto str = to_string(buffer.GetData());
        std::cout << str << std::endl;
        assert(str == "abcdefghijklmnopqrst");

        buffer.Consume(3);
        str = to_string(buffer.GetData());
        std::cout << str << std::endl;
        assert(str == "defghijklmnopqrst");
    }
    {
        wsocket::SlidingBuffer buffer;
        char                   data[] = "abcdefghijklmnopqrst";
        buffer.Feed({reinterpret_cast<uint8_t *>(data), 20});
        auto str = to_string(buffer.GetData());
        std::cout << str << std::endl;
        assert(str == "abcdefghijklmnopqrst");

        buffer.Consume(4, 3);
        str = to_string(buffer.GetData());
        std::cout << str << std::endl;
        assert(str == "abcdhijklmnopqrst");

        buffer.Consume(3, 14);
        str = to_string(buffer.GetData());
        std::cout << str << std::endl;
        assert(str == "abc");
    }
}

class Client : public wsocket::WSocketContext::Listener {
public:
    void OnError(std::error_code code) override {}

    void OnConnected() override { std::cout << "OnConnected" << std::endl; }
    void OnClose(int16_t code, const std::string &reason) override {
        std::cout << "OnClose: " << code << ":" << reason << std::endl;
    }
    void OnPing() override { std::cout << "OnPing" << std::endl; }
    void OnPong() override { std::cout << "OnPong" << std::endl; }

    void OnText(std::string_view text, bool finish) override {
        std::cout << "OnText:" << std::string(text) << std::endl;
    }
    void OnBinary(wsocket::Buffer buffer, bool finish) override {
        std::cout << "OnBinary:" << std::string(reinterpret_cast<char *>(buffer.buf), buffer.size) << std::endl;
    }
};

void test_WSocketContext() {
    std::cout << "================== test_WSocketContext ==================" << std::endl;
    wsocket::WSocketContext ctx1;
    wsocket::WSocketContext ctx2;

    Client client1;
    ctx1.AddListener(&client1);
    Client client2;
    ctx2.AddListener(&client2);

    ctx1.ResetSendHandler([&](wsocket::Buffer buffer) { ctx2.Feed(buffer); });
    ctx2.ResetSendHandler([&](wsocket::Buffer buffer) { ctx1.Feed(buffer); });

    ctx1.Handshake();

    ctx1.SendText("abcdefghijklmnopqrst");
    ctx2.SendBinary(wsocket::Buffer({reinterpret_cast<uint8_t *>(const_cast<char *>("zxc")), 3}));

    ctx1.Ping();
    ctx2.Ping();
    ctx1.Pong();
    ctx2.Pong();

    ctx1.SendText("abcdefghijklmnopqrst");
    ctx2.SendBinary(wsocket::Buffer({reinterpret_cast<uint8_t *>(const_cast<char *>("zxc")), 3}));

    ctx1.SendText("abcdefghijklmnopqrst");
    ctx2.SendBinary(wsocket::Buffer({reinterpret_cast<uint8_t *>(const_cast<char *>("zxc")), 3}));

    ctx2.Close(wsocket::CloseCode::CLOSE_NORMAL);
    std::cout << "================== test_WSocketContext ==================" << std::endl;
}

class TestWSocket : public wsocket::WSocket {
protected:
    explicit TestWSocket(const asio::any_io_executor &io_executor) : wsocket::WSocket(io_executor) {}
    explicit TestWSocket(asio::ip::tcp::socket &&socket) : wsocket::WSocket(std::move(socket)) {}

public:
    static std::shared_ptr<TestWSocket> Create(asio::any_io_executor io_executor) {
        return std::shared_ptr<TestWSocket>(new TestWSocket(std::move(io_executor)));
    }
    static std::shared_ptr<TestWSocket> Create(asio::ip::tcp::socket &&socket) {
        return std::shared_ptr<TestWSocket>(new TestWSocket(std::move(socket)));
    }

    ~TestWSocket() override {}

private:
    void OnError(std::error_code code) override { std::cout << "OnError: " << code << std::endl; }
    void OnConnected() override { std::cout << "OnConnected" << std::endl; }
    void OnClose(int16_t code, const std::string &reason) override {
        std::cout << "OnClose: " << code << ":" << reason << std::endl;
    }
    void OnPing() override {
        std::cout << "OnPing" << std::endl;
        wsocket::WSocket::OnPing();
        this->Text("hello");
    }
    void OnPong() override { std::cout << "OnPong" << std::endl; }
    void OnText(std::string_view text, bool finish) override { std::cout << "OnText:" << text << std::endl; }
    void OnBinary(wsocket::Buffer buffer, bool finish) override {
        std::cout << "OnBinary:" << std::string(reinterpret_cast<char *>(buffer.buf), buffer.size) << std::endl;
    }
};

void test_asio_wsocket() {
    std::cout << "================== test_asio_wsocket ==================" << std::endl;
    asio::io_context io_executor;

    using tcp = asio::ip::tcp;
    tcp::acceptor server(io_executor, asio::ip::tcp::v4());
    server.bind(tcp::endpoint(asio::ip::tcp::v4(), 12000));
    server.listen();
    server.async_accept([&](asio::error_code ec, asio::ip::tcp::socket peer) {
        if(ec) {
            std::cout << ec.message() << std::endl;
            return;
        }

        std::cout << peer.remote_endpoint().address().to_string() << ":" << peer.remote_endpoint().port() << std::endl;
        auto cli = TestWSocket::Create(std::move(peer));
        cli->Start();
    });


    auto client = TestWSocket::Create(io_executor.get_executor());
    client->Handshake(asio::ip::tcp::endpoint(asio::ip::make_address_v4("127.0.0.1"), 12000));

    io_executor.run();
    std::cout << "================== test_asio_wsocket ==================" << std::endl;
}

#ifdef ASIO_HAS_LOCAL_SOCKETS
class TestUnixWSocket : public wsocket::UnixWSocket {
protected:
    explicit TestUnixWSocket(const asio::any_io_executor &io_executor) : wsocket::UnixWSocket(io_executor) {}
    explicit TestUnixWSocket(asio::local::stream_protocol::socket &&socket) : wsocket::UnixWSocket(std::move(socket)) {}

public:
    static std::shared_ptr<TestUnixWSocket> Create(asio::any_io_executor io_executor) {
        return std::shared_ptr<TestUnixWSocket>(new TestUnixWSocket(std::move(io_executor)));
    }
    static std::shared_ptr<TestUnixWSocket> Create(asio::local::stream_protocol::socket &&socket) {
        return std::shared_ptr<TestUnixWSocket>(new TestUnixWSocket(std::move(socket)));
    }

    ~TestUnixWSocket() override {}

private:
    void OnError(std::error_code code) override {
        std::cout << "OnError: " << code << ":" << code.message() << std::endl;
    }
    void OnConnected() override { std::cout << "OnConnected" << std::endl; }
    void OnClose(int16_t code, const std::string &reason) override {
        std::cout << "OnClose: " << code << ":" << reason << std::endl;
    }
    void OnPing() override {
        std::cout << "OnPing" << std::endl;
        wsocket::UnixWSocket::OnPing();
        this->Text("hello");
    }
    void OnPong() override { std::cout << "OnPong" << std::endl; }
    void OnText(std::string_view text, bool finish) override { std::cout << "OnText:" << text << std::endl; }
    void OnBinary(wsocket::Buffer buffer, bool finish) override {
        std::cout << "OnBinary:" << std::string(reinterpret_cast<char *>(buffer.buf), buffer.size) << std::endl;
    }
};
void test_asio_unix_wsocket() {
    asio::io_context io_executor;

    using unix_socket = asio::local::stream_protocol;
    unix_socket::acceptor server(io_executor);
    server.open();

#ifdef _WIN32
    std::string path = "H:/Code/CLion/WSocket/test_unix_wsocket.sock";
#else
    std::string path = "/tmp/test_unix_wsocket.sock";
#endif

    std::remove(path.c_str());

    asio::error_code ec;
    server.bind(unix_socket::endpoint(path), ec);
    if(ec) {
        std::cout << "bind error: " << ec.message() << std::endl;
        return;
    }
    server.listen();

    server.async_accept([&](asio::error_code ec, unix_socket::socket peer) {
        if(ec) {
            std::cout << "accept error: " << ec.message() << std::endl;
            return;
        }

        std::cout << "remote path:" << peer.remote_endpoint().path() << std::endl;
        auto cli = TestUnixWSocket::Create(std::move(peer));
        cli->Start();
    });


    auto client = TestUnixWSocket::Create(io_executor.get_executor());
    client->Handshake(unix_socket::endpoint(path));

    io_executor.run();
    server.close();
    std::remove(path.c_str());
}
#endif

int main() {
    std::cout << "================== start ==================" << std::endl;
    try {
        testBasicHeader();
        testFrameHeader();
        test_SlidingBuffer();
        test_WSocketContext();
        // test_asio_wsocket();
        test_asio_unix_wsocket();
    } catch(const std::exception &e) {
        std::cout << "exception: " << e.what() << std::endl;
    }
    std::cout << "================== end ==================" << std::endl;
    return 0;
}
