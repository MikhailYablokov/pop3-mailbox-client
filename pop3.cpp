#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>  // Для Sleep

using namespace std;

// Функция для получения полного ответа от сервера (до "." для многострочных)
string receive_full_response(SOCKET sockfd) {
    string response;
    char buffer[1024];
    while (true) {
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) break;
        buffer[bytes_received] = '\0';
        response += buffer;

        // Проверяем на конец многострочного ответа (строка с "." )
        size_t pos = response.rfind("\r\n.\r\n");
        if (pos != string::npos) {
            response = response.substr(0, pos + 3);  // Обрезаем до "."
            break;
        }
    }
    return response;
}

// Функция отправки команды и получения ответа
string send_command(SOCKET sockfd, const string& command, bool multiline = false) {
    send(sockfd, (command + "\r\n").c_str(), command.length() + 2, 0);
    if (multiline) {
        return receive_full_response(sockfd);
    } else {
        char buffer[1024];
        int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        buffer[bytes_received] = '\0';
        return string(buffer);
    }
}

// Основная функция для получения почты (RETR)
void retrieve_message(SOCKET sockfd, int msg_num) {
    string response = send_command(sockfd, "RETR " + to_string(msg_num), true);
    if (response.find("+OK") != string::npos) {
        // Убираем статусную строку +OK
        size_t body_start = response.find("\r\n");
        if (body_start != string::npos) {
            string message_body = response.substr(body_start + 2);
            cout << "Сообщение получено:\n" << message_body << endl;
        }
    } else {
        cout << "Ошибка получения сообщения: " << response << endl;
    }
}

// Задержка
void add_delay() {
    Sleep(1000);
}

void print_menu() {
    cout << "\nВыберите команду:\n";
    cout << "1. STAT (Статистика)\n";
    cout << "2. LIST (Список сообщений)\n";
    cout << "3. RETR (Получить сообщение)\n";
    cout << "4. DELE (Удалить сообщение)\n";
    cout << "5. RSET (Сброс удалений)\n";
    cout << "6. NOOP (Ничего не делать)\n";
    cout << "7. QUIT (Завершить сессию)\n";
    cout << "8. EXIT (Выход из программы)\n";
    cout << "Введите выбор (1-8): ";
}

int main(int argc, char* argv[]) {
    string username = "user123@localdomain.com";
    string password = "12345678";
    string server_ip = "127.0.0.1";
    int port = 110;

    // Парсинг аргументов
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--help") {
            cout << "Usage: pop3_client [options]\n";
            return 0;
        } else if (arg == "--user" && i + 1 < argc) {
            username = argv[++i];
        } else if (arg == "--pass" && i + 1 < argc) {
            password = argv[++i];
        } else if (arg == "--server" && i + 1 < argc) {
            server_ip = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = stoi(argv[++i]);
        } else {
            cerr << "Неизвестная опция: " << arg << "\n";
            return 1;
        }
    }

    // Winsock init
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return 1;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета!" << endl;
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Ошибка подключения!" << endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Авторизация
    add_delay();
    cout << "Отправка USER" << endl;
    send_command(sockfd, "USER " + username);

    add_delay();
    cout << "Отправка PASS" << endl;
    send_command(sockfd, "PASS " + password);

    string response = send_command(sockfd, "STAT");
    if (response.find("+OK") != string::npos) {
        cout << "\nЯщик готов\n";
        while (true) {
            print_menu();
            int choice;
            cin >> choice;

            switch (choice) {
                case 1:
                    cout << send_command(sockfd, "STAT") << endl;
                    break;
                case 2:
                    cout << send_command(sockfd, "LIST", true) << endl;
                    break;
                case 3: {
                    cout << "Введите номер сообщения: ";
                    int msg_num;
                    cin >> msg_num;
                    retrieve_message(sockfd, msg_num);
                    break;
                }
                case 4: {
                    cout << "Введите номер для удаления: ";
                    int msg_num;
                    cin >> msg_num;
                    cout << send_command(sockfd, "DELE " + to_string(msg_num)) << endl;
                    break;
                }
                case 5:
                    cout << send_command(sockfd, "RSET") << endl;
                    break;
                case 6:
                    cout << send_command(sockfd, "NOOP") << endl;
                    break;
                case 7:
                    send_command(sockfd, "QUIT");
                    closesocket(sockfd);
                    WSACleanup();
                    return 0;
                case 8:
                    cout << "Выход...\n";
                    closesocket(sockfd);
                    WSACleanup();
                    return 0;
                default:
                    cout << "Неверный выбор.\n";
                    break;
            }
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
