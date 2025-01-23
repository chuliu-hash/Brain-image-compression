#include"streambuffer.h"

// 重写 streambuf 的 overflow 方法
std::streambuf::int_type StreamBuffer::overflow(std::streambuf::int_type v) {
    if (v == '\n') {
        // 将内容写入 QPlainTextEdit
        QMetaObject::invokeMethod(m_textEdit, "appendPlainText", Qt::QueuedConnection,
            Q_ARG(QString, QString::fromStdString(m_buffer)));
        m_buffer.clear();
    }
    else {
        m_buffer += static_cast<char>(v);
    }
    return v;
}

// 重写 sync 方法
int StreamBuffer::sync() {
    // 刷新缓冲区
    if (!m_buffer.empty()) {
        QMetaObject::invokeMethod(m_textEdit, "appendPlainText", Qt::QueuedConnection,
            Q_ARG(QString, QString::fromStdString(m_buffer)));
        m_buffer.clear();
    }
    return 0;
}