#pragma once

#include <iostream>
#include <streambuf>
#include <QPlainTextEdit>
#include <QTextStream>

class StreamBuffer : public std::streambuf {
public:
    StreamBuffer(QPlainTextEdit* textEdit) : m_textEdit(textEdit) {}

protected:
    // 重写 streambuf 的 overflow 方法
    int_type overflow(int_type v) override;
        

    // 重写 sync 方法
    int sync() override;
       

private:
    QPlainTextEdit* m_textEdit;  // 目标 QPlainTextEdit
    std::string m_buffer;        // 缓冲区
};
