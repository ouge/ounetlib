#ifndef NET_TCPCONNECTION_H
#define NET_TCPCONNECTION_H

#include "base/Copyable.h"
#include "base/StringPiece.h"
#include "base/Types.h"
#include "net/Buffer.h"
#include "net/Callbacks.h"
#include "net/InetAddress.h"

#include <boost/any.hpp>
#include <memory>

struct tcp_info;

namespace ouge {
namespace net {
class EventLoop;
class Channel;
class Socket;

class TcpConnection : NonCopyable,
                      public std::enable_shared_from_this<TcpConnection> {
  public:
    TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop*         getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool               connected() const { return state_ == kConnected; }
    bool               disconnected() const { return state_ == kDisconnected; }

    bool        getTcpInfo(struct tcp_info*) const;
    std::string getTcpInfoString() const;

    void send(std::string&& message);
    void send(const void* message, int len);
    void send(const StringPiece& message);
    void send(Buffer&& message);
    void send(Buffer* message);
    void shutdown();

    void forceClose();
    void forceCloseWithDelay(double seconds);

    void setTcpNoDelay(bool on);

    void startRead();
    void stopRead();
    bool isReading() const { return reading_; };

    void setContext(const boost::any& context) { context_ = context; }

    const boost::any& getContext() const { return context_; }

    boost::any* getMutableContext() { return &context_; }

    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                  size_t                       highWaterMark) {
        highWaterMarkCallback_ = cb;
        highWaterMark_         = highWaterMark;
    }

    Buffer* inputBuffer() { return &inputBuffer_; }

    Buffer* outputBuffer() { return &outputBuffer_; }

    // TcpServer 和 TcpClient 使用
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void connectEstablished();

    void connectDestroyed();

  private:
    enum StateE {
        kDisconnected,    // 未连接
        kConnecting,      // 正在连接
        kConnected,       // 已连接
        kDisconnecting    // 正在断开
    };

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    // void sendInLoop(string&& message);
    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    // void shutdownAndForceCloseInLoop(double seconds);
    void forceCloseInLoop();
    void setState(StateE s) { state_ = s; }
    const char*          stateToString() const;
    void                 startReadInLoop();
    void                 stopReadInLoop();

    EventLoop*        loop_;
    const std::string name_;
    StateE            state_;    // FIXME: use atomic variable
    bool              reading_;
    // we don't expose those classes to client.
    std::unique_ptr<Socket>  socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress        localAddr_;
    const InetAddress        peerAddr_;
    ConnectionCallback       connectionCallback_;
    MessageCallback          messageCallback_;
    WriteCompleteCallback    writeCompleteCallback_;
    HighWaterMarkCallback    highWaterMarkCallback_;
    CloseCallback            closeCallback_;
    size_t                   highWaterMark_;
    Buffer                   inputBuffer_;
    Buffer                   outputBuffer_;
    boost::any               context_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
}
}

#endif