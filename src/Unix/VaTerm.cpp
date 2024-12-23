#pragma once
/*
 * (c) 2024 Lc3124
 * LICENSE (MIT)
 * VaTerm在linux下的实现
 */

#include "VaTerm.hpp"

// std
#include <cstdio>
#include <cstring>
#include <iostream>
// sys
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>


termios originalAttrs;
termios currentAttrs;

VaTerm::VaTerm() {
    tcgetattr(STDIN_FILENO, &originalAttrs);
    currentAttrs = originalAttrs;
}

//释放时会恢复终端设置
VaTerm::~VaTerm() {
    tcsetattr(STDIN_FILENO, TCSANOW, &originalAttrs);
}


// Clear the entire screen.
const char* VaTerm:: _Clear()
{
    return "\033[2J";
}
void VaTerm::Clear()
{
    fastOutput("\033[2J");
}

// Clear the area from the cursor's position to the end of the line.
const char* VaTerm::_ClearLine()
{
    return "\033[K";
}
void VaTerm::ClearLine()
{
    fastOutput("\033[K");
}
void VaTerm::getTerminalAttributes() {
    tcgetattr(STDIN_FILENO, &currentAttrs);
}

void VaTerm::setTerminalAttributes(const termios &newAttrs) {
    currentAttrs = newAttrs;
    tcsetattr(STDIN_FILENO, TCSANOW, &currentAttrs);
}

void VaTerm::enableEcho() {
    currentAttrs.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &currentAttrs);
}

void VaTerm::disableEcho() {
    currentAttrs.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &currentAttrs);
}

void VaTerm::enableConsoleBuffering() {
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_SYNC);
}

void VaTerm::disableConsoleBuffering() {
    int flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_SYNC);
}

void VaTerm::getTerminalSize(int &rows, int &cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    rows = w.ws_row;
    cols = w.ws_col;
}

void VaTerm::setCursorPosition(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

void VaTerm::saveCursorPosition() { std::cout << "\033[s"; }

void VaTerm::restoreCursorPosition() { std::cout << "\033[u"; }

void VaTerm::fastOutput(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
}

char VaTerm:: nonBufferedGetKey() {
    struct termios oldt, newt;
    char c;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    read(STDIN_FILENO, &c, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
}

const char* VaTerm::getTerminalType() { return std::getenv("TERM"); }


void VaTerm::setLineBuffering(bool enable) {
    if (enable) {
        currentAttrs.c_lflag |= ICANON;
    } else {
        currentAttrs.c_lflag &= ~ICANON;
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &currentAttrs);
}

//禁止回显，然后阻塞，返回获取到的字符，类似于getch()
char VaTerm::getCharacter() {
    disableEcho();
    char c = nonBufferedGetKey();
    enableEcho();
    return c;
}

//判断终端是否支持某一功能 
bool VaTerm::isTerminalFeatureSupported(const char *feature) {
    const char *termType = getTerminalType();
    if (termType == nullptr) {
        return false;

    }
    if (strstr(termType, feature)!= nullptr) {
        return true;
    }
    return false;
}

// 设置字符输入延迟
void VaTerm::setCharacterDelay(int milliseconds) {
    termios newAttrs = currentAttrs;
    newAttrs.c_cc[VMIN] = 0;
    newAttrs.c_cc[VTIME] = milliseconds / 100;
    setTerminalAttributes(newAttrs);

}

// 获取输入速度
int VaTerm::getInputSpeed() {
    speed_t speed;
    tcgetattr(STDIN_FILENO, &currentAttrs);
    speed = cfgetospeed(&currentAttrs);
    return static_cast<int>(speed);

}

// 设置输入速度
void VaTerm::setInputSpeed(int speed) {
    termios newAttrs = currentAttrs;
    cfsetospeed(&newAttrs, static_cast<speed_t>(speed));
    cfsetispeed(&newAttrs, static_cast<speed_t>(speed));
    setTerminalAttributes(newAttrs);

}

// 设置输出速度
void VaTerm::setOutputSpeed(int speed) {
    termios newAttrs = currentAttrs;
    cfsetospeed(&newAttrs, static_cast<speed_t>(speed));
    setTerminalAttributes(newAttrs);

}

int VaTerm::keyPressed(char &k) {
    struct termios oldt, newt;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    char c;
    int res = read(STDIN_FILENO, &c, 1);
    if (res > 0) {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        k=c;
        return 1;
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        k=static_cast<char>(-1);
        return -0;
    }
}

// 函数用于设置光标形状
void VaTerm::setCursorShape(CursorShape shape) {
    termios newAttrs = currentAttrs;

    // 根据传入的光标形状设置相应的c_cflag值
    switch (shape) {
        case CURSOR_BLOCK:
            newAttrs.c_cflag &= ~(ECHOCTL);
            break;
        case CURSOR_UNDERLINE:
            newAttrs.c_cflag |= (ECHOCTL | ECHOE);
            break;
        case CURSOR_VERTICAL_BAR:
            newAttrs.c_cflag |= ECHOCTL;
            break;
    }
    // 设置新的终端属性
    setTerminalAttributes(newAttrs);
}
