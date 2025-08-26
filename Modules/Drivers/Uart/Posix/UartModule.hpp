#ifndef __UART_MODULE_HPP__
#define __UART_MODULE_HPP__

//https://en.wikibooks.org/wiki/Serial_Programming/termios

#include "UartAbstraction.hpp"
//Posix
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
//C++
#include <cstring>

class Uart : public UartAbstraction {
    public:
    Uart() : UartAbstraction() {}

    ErrorType init() override;
    ErrorType deinit() override;
    ErrorType txBlocking(const std::string &data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType rxBlocking(std::string &buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params) override;
    ErrorType txNonBlocking(const std::shared_ptr<std::string> data, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, const Bytes bytesWritten)> callback) override;
    ErrorType rxNonBlocking(std::shared_ptr<std::string> buffer, const Milliseconds timeout, const IcCommunicationProtocolTypes::AdditionalCommunicationParameters &params, std::function<void(const ErrorType error, std::shared_ptr<std::string> buffer)> callback) override;
    ErrorType flushRxBuffer() override;

    private:
    /// @brief The discovered serial device path
    std::string _devicePath;
    /// @brief The file descriptor
    int _fileDescriptor = -1;

    /**
     * @brief Convert a peripheral number to a device path
     * @param peripheralNumber The peripheral number to convert
     * @returns The device path
     */
    std::string toDevicePath(const PeripheralNumber peripheralNumber) {
        switch (peripheralNumber) {
            case PeripheralNumber::Zero:
                return "/dev/cu.usbserial-10";
            case PeripheralNumber::One:
                return "/dev/tty.usbserial-10";
            case PeripheralNumber::Two:
                return "/dev/tty.usbmodem-101";
            case PeripheralNumber::Three:
                return "/dev/tty.usbserial-1110";
            case PeripheralNumber::Four:
                return "/dev/ttyUSB0";
            case PeripheralNumber::Five:
                return "/dev/ttyUSB1";
            case PeripheralNumber::Six:
                return "/dev/ttyUSB2";
            case PeripheralNumber::Seven:
                return "/dev/ttyACM0";
            case PeripheralNumber::Eight:
                return "/dev/ttyACM1";
            case PeripheralNumber::Nine:
                return "/dev/ttyACM2";
            default:
                    return "";
        }
    }

    /**
     * @brief Convert a baud rate to a POSIX baud rate
     * @param baudRate The baud rate to convert
     * @returns The POSIX baud rate
     */
    ErrorType toPosixBaudRate(const uint32_t baudRate, struct termios &tty) {
        ErrorType error = ErrorType::Success;
        speed_t posixBaudRate;

        switch (baudRate) {
            case 9600:
                posixBaudRate = B9600;
                break;
            case 19200:
                posixBaudRate = B19200;
                break;
            case 38400:
                posixBaudRate = B38400;
                break;
            case 57600:
                posixBaudRate = B57600;
                break;
            case 115200:
                posixBaudRate = B115200;
                break;
            case 230400:
                posixBaudRate = B230400;
                break;
            default:
                return ErrorType::InvalidParameter;
        }

        cfsetospeed(&tty, posixBaudRate);
        cfsetispeed(&tty, posixBaudRate);

        return error;
    }

    /**
     * @brief Convert a data bits to a POSIX data bits
     * @param dataBits The data bits to convert
     * @returns The POSIX data bits
     */
    ErrorType toPosixDataBits(const uint8_t dataBits, struct termios &tty) {
        ErrorType error = ErrorType::Success;

        switch (dataBits) {
            case 5:
                tty.c_cflag |= CS5;
                break;
            case 6:
                tty.c_cflag |= CS6;
                break;
            case 7:
                tty.c_cflag |= CS7;
                break;
            case 8:
                tty.c_cflag |= CS8;
                break;
            default:
                error = ErrorType::InvalidParameter;
        }

        return error;
    }

    /**
     * @brief Convert a parity to a POSIX parity
     * @param parity The parity to convert
     * @returns The POSIX parity
     */
    ErrorType toPosixParity(const char parity, struct termios &tty) {
        ErrorType error = ErrorType::Success;

        switch (parity) {
            case 'N':
                tty.c_cflag &= ~PARENB;
                break;
            case 'O':
                tty.c_cflag |= PARENB | PARODD;
                break;
            case 'E':
                tty.c_cflag |= PARENB;
                break;
            default:
                error = ErrorType::InvalidParameter;
        }

        return error;
    }

    /**
     * @brief Convert a flow control to a POSIX flow control
     * @param flowControl The flow control to convert
     * @param tty The termios config to set
     * @returns The error type
     */
    ErrorType toPosixFlowControl(const UartTypes::FlowControl flowControl, struct termios &tty) {
        ErrorType error = ErrorType::Success;

        switch (flowControl) {
            case UartTypes::FlowControl::Disable:
                tty.c_iflag &= ~(IXON | IXOFF | IXANY);
                break;
            case UartTypes::FlowControl::Rts:
                tty.c_iflag |= IXOFF;
                break;
            case UartTypes::FlowControl::Cts:
                tty.c_iflag |= IXON;
                break;
            case UartTypes::FlowControl::CtsRts:
                tty.c_iflag |= IXON | IXOFF;
                break;
            default:
                error = ErrorType::InvalidParameter;
        }

        return error;
    }
    /**
     * @brief Convert a stop bits to a POSIX stop bits
     * @param stopBits The stop bits to convert
     * @returns The POSIX stop bits
     */
    ErrorType toPosixStopBits(const uint8_t stopBits, struct termios &tty) {
        ErrorType error = ErrorType::Success;

        switch (stopBits) {
            case 1:
                tty.c_cflag &= ~CSTOPB;
                break;
            case 2:
                tty.c_cflag |= CSTOPB;
                break;
            default:
                error = ErrorType::InvalidParameter;
        }

        return error;
    }

    /**
     * @brief Set the termio config
     * @param tty The termio config to set
     */
    ErrorType setTermiosConfig(struct termios &tty) {
        memset(&tty, 0, sizeof(tty));
        ErrorType error = ErrorType::Success;

        error = toPosixBaudRate(uartParams().driverConfig.baudRate, tty); 
        if (error == ErrorType::Success) {
            error = toPosixDataBits(uartParams().driverConfig.dataBits, tty); 

            if (ErrorType::Success == error) {
                error = toPosixParity(uartParams().driverConfig.parity, tty);

                if (error == ErrorType::Success) {
                    error = toPosixStopBits(uartParams().driverConfig.stopBits, tty);

                    if (error == ErrorType::Success) {
                        error = toPosixFlowControl(uartParams().driverConfig.flowControl, tty);

                        if (error == ErrorType::Success) {
                            tty.c_cc[VMIN]  = 0;
                            tty.c_cc[VTIME] = 5;
                            tty.c_cflag |= (CLOCAL | CREAD);

                            if (tcsetattr(_fileDescriptor, TCSANOW, &tty) != 0) {
                                close(_fileDescriptor);
                                _fileDescriptor = -1;
                                error = ErrorType::Failure;
                            }
                        }
                    }
                }
            }
        }

        return error;
    }
};

#endif // __UART_MODULE_HPP__