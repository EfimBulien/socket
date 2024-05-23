#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;

int main()
{
    WSADATA wsaData; // Эта структура содержит информацию о реализации Windows Sockets
    ADDRINFO* addResult; // Этот указатель будет хранить результаты адресной информации
    ADDRINFO hints; // Эта структура будет хранить подсказки для функции getaddrinfo
    SOCKET ConnectSocket = INVALID_SOCKET; // Это будет наш сокет для подключения к серверу
    const char* sendBuffer = "Hello from Client"; // Первое сообщение для отправки на сервер
    const char* secondMessage = "Second message from Client"; // Второе сообщение для отправки на сервер
    char recvBuffer[512]; // Буфер для хранения полученных данных от сервера

    // Инициализация Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cout << "WSAStartup failes result" << std::endl;
        return 1;
    }

    // Настраиваем структуру hints, чтобы указать тип соединения, который нам нужен
    ZeroMemory(&hints, sizeof(hints)); // Обнуляем память для hints
    hints.ai_family = AF_INET; // Используем IPv4
    hints.ai_socktype = SOCK_STREAM; // Тип сокета потоковый (TCP)
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP

    // Определяем адрес и порт сервера
    result = getaddrinfo("localhost", "666", &hints, &addResult);
    if (result != 0)
    {
        std::cout << "Getaddrinfo wit error" << std::endl;
        WSACleanup();
        return 1;
    }

    // Создаем сокет для подключения к серверу
    ConnectSocket = socket(addResult->ai_family, addResult->ai_socktype, addResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cout << "Socket creation with " << std::endl;
        WSACleanup();
        return 1;
    }

    // Подключаемся к серверу
    result = connect(ConnectSocket, addResult->ai_addr, (int)addResult->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        std::cout << "Unable to connect to server" << std::endl;
        closesocket(ConnectSocket); // Закрываем сокет
        freeaddrinfo(addResult); // Освобождаем адресную информацию
        WSACleanup();
        return 1;
    }

    // Отправляем первое сообщение на сервер
    result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), MSG_DONTROUTE);

    if (result == SOCKET_ERROR) 
    {
        std::cout << "Send failed with error" << std::endl;
        // Очищаем и закрываем сокет
        closesocket(ConnectSocket);
        freeaddrinfo(addResult);
        WSACleanup();
        return 1;
    }

    std::cout << "First message sent successfully" << std::endl;

    // Отправляем второе сообщение на сервер
    result = send(ConnectSocket, secondMessage, (int)strlen(secondMessage), MSG_DONTROUTE);

    if (result == SOCKET_ERROR) 
    {
        std::cout << "Send failed with error" << std::endl;
        // Очищаем и закрываем сокет
        closesocket(ConnectSocket);
        freeaddrinfo(addResult);
        WSACleanup(); //очситка WSAStartup
        return 1;
    }

    std::cout << "Second message sent successfully" << std::endl;

    // Получаем данные от сервера до тех пор, пока сервер не закроет соединение
    do
    {
        ZeroMemory(recvBuffer, 512); // Обнуляем буфер приема
        result = recv(ConnectSocket, recvBuffer, 512, 0); // Получаем данные от сервера
        if (result > 0)
        {
            cout << "Received " << result << "bytes" << endl;
            cout << "Received data " << recvBuffer << endl;
        }
        else if (result == 0)
        {
            cout << "Connection closed" << endl; // Соединение закрыто сервером
        }
        else
        {
            cout << "Received failed " << endl;
            closesocket(ConnectSocket); // Закрываем сокет при ошибке
        }
    } while (result > 0);

    // Завершаем соединение для отправки, так как больше данных отправлять не будем
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        cout << "shutdown error " << endl;
        freeaddrinfo(addResult);
        WSACleanup(); //очситка WSAStartup
        return 1;
    }

    // Очищаем и закрываем сокет
    closesocket(ConnectSocket);
    freeaddrinfo(addResult);
    WSACleanup(); //очситка WSAStartup
    return 0;
}
