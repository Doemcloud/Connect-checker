#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <thread>
#include <ctime>

#define PORT 8080
#define LOG_FILE "server_log.txt"
#define BUFFER_SIZE 1024

using boost::asio::ip::tcp;

// Функция для записи в лог
void log_message(const std::string& message) {
    std::ofstream log_file(LOG_FILE, std::ios_base::out | std::ios_base::app);
    if (log_file.is_open()) {
        std::time_t now = std::time(nullptr);
        char time_str[100];
        if (std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now))) {
            log_file << "[" << time_str << "] " << message << std::endl;
        }
        log_file.close();
    } else {
        std::cerr << "Unable to open log file." << std::endl;
    }
}

// Функция для обработки соединения с клиентом
void handle_client(std::shared_ptr<tcp::socket> socket) {
    try {
        // Получаем IP-адрес клиента
        std::string client_ip = socket->remote_endpoint().address().to_string();
        std::cout << "Connection from: " << client_ip << std::endl;
        log_message("Connection from: " + client_ip);

        // Обработка данных от клиента
        char buffer[BUFFER_SIZE];
        boost::system::error_code error;
        while (true) {
            std::size_t length = socket->read_some(boost::asio::buffer(buffer), error);
            if (error == boost::asio::error::eof)
                break; // Соединение закрыто клиентом
            else if (error)
                throw boost::system::system_error(error); // Другие ошибки

            std::string received_data(buffer, length);
            std::cout << "Received from " << client_ip << ": " << received_data << std::endl;
            log_message("Received from " + client_ip + ": " + received_data);

            // Эхо
            boost::asio::write(*socket, boost::asio::buffer(buffer, length));
        }
    } catch (std::exception& e) {
        log_message("Exception in connection handling: " + std::string(e.what()));
    }

    // Закрываем соединение
    socket->close();
    log_message("Connection closed");
}

int main() {
    try {
        boost::asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), PORT));
        log_message("Server is listening on port " + std::to_string(PORT));
        std::cout << "Listening on port " << PORT << std::endl;

        while (true) {
            std::shared_ptr<tcp::socket> socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);

            std::thread(client_thread, handle_client, socket).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        log_message("Exception: " + std::string(e.what()));
    }

    return 0;
}
