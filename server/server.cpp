#define WIN32_LEAN_AND_MEAN 

#include <iostream> 
#include <Windows.h> 
#include <WinSock2.h> 
#include <WS2tcpip.h> 

int main()
{
    WSADATA wsaData; // Структура для хранения информации о реализации Windows Sockets
    ADDRINFO* addResult = nullptr; // Указатель для хранения результатов getaddrinfo
    ADDRINFO hints; // Структура для хранения информации о параметрах запроса
    SOCKET ClientSocket = INVALID_SOCKET; // Сокет для подключения клиента
    SOCKET ListenSocket = INVALID_SOCKET; // Сокет для прослушивания входящих соединений

    const char* sendBuffer = "Hello from server"; // Буфер для отправки данных клиенту
    char recvBuffer[512]; // Буфер для получения данных от клиента

    // Инициализируем использование Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cout << "WSAStartup failed with result " << result << std::endl;
        return 1; // Завершаем программу с кодом ошибки
    }

    ZeroMemory(&hints, sizeof(hints)); // Обнуляем структуру hints
    hints.ai_family = AF_INET; // Используем IPv4
    hints.ai_socktype = SOCK_STREAM; // Используем потоковый сокет (TCP)
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP
    hints.ai_flags = AI_PASSIVE; // Используем сокет для прослушивания

    // Получаем информацию о адресе для привязки сокета
    result = getaddrinfo(NULL, "666", &hints, &addResult);
    if (result != 0)
    {
        std::cout << "Getaddrinfo failed with error " << result << std::endl;
        WSACleanup(); // Чистим ресурсы Winsock
        return 1; // Завершаем программу с кодом ошибки
    }

    // Создаем сокет для прослушивания входящих соединений
    ListenSocket = socket(addResult->ai_family, addResult->ai_socktype, addResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation failed" << std::endl;
        freeaddrinfo(addResult); // Освобождаем результаты getaddrinfo
        WSACleanup(); // Чистим ресурсы Winsock
        return 1; // Завершаем программу с кодом ошибки
    }

    // Привязываем сокет к адресу
    result = bind(ListenSocket, addResult->ai_addr, (int)addResult->ai_addrlen);
    if (result == SOCKET_ERROR)
    {
        std::cout << "Binding socket failed" << std::endl;
        closesocket(ListenSocket); // Закрываем сокет
        freeaddrinfo(addResult); // Освобождаем результаты getaddrinfo
        WSACleanup(); // Чистим ресурсы Winsock
        return 1; // Завершаем программу с кодом ошибки
    }

    // Переводим сокет в режим прослушивания
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cout << "Listening on socket failed" << std::endl;
        closesocket(ListenSocket); // Закрываем сокет
        freeaddrinfo(addResult); // Освобождаем результаты getaddrinfo
        WSACleanup(); // Чистим ресурсы Winsock
        return 1; // Завершаем программу с кодом ошибки
    }

    // Принимаем входящее соединение
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        std::cout << "Accepting connection failed" << std::endl;
        closesocket(ListenSocket); // Закрываем сокет
        freeaddrinfo(addResult); // Освобождаем результаты getaddrinfo
        WSACleanup(); // Чистим ресурсы Winsock
        return 1; // Завершаем программу с кодом ошибки
    }

    // Закрываем сокет прослушивания, так как он больше не нужен
    closesocket(ListenSocket);

    // Основной цикл для приема и отправки данных
    do
    {
        ZeroMemory(recvBuffer, 512); // Обнуляем буфер приема
        result = recv(ClientSocket, recvBuffer, 512, 0); // Принимаем данные от клиента
        if (result > 0)
        {
            std::cout << "Received " << result << " bytes" << std::endl;
            std::cout << "Received data: " << recvBuffer << std::endl;

            // Отправляем данные обратно клиенту
            result = send(ClientSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR)
            {
                std::cout << "Failed to send data back" << std::endl;
                closesocket(ClientSocket); // Закрываем клиентский сокет
                freeaddrinfo(addResult); // Освобождаем результаты getaddrinfo
                WSACleanup(); // Чистим ресурсы Winsock
                return 1; // Завершаем программу с кодом ошибки
            }
        }
        else if (result == 0)
        {
            std::cout << "Connection closing..." << std::endl;
        }
        else
        {
            std::cout << "recv failed with error " << WSAGetLastError() << std::endl;
            closesocket(ClientSocket); // Закрываем клиентский сокет
            freeaddrinfo(addResult); // Освобождаем результаты getaddrinfo
            WSACleanup(); // Чистим ресурсы Winsock
            return 1; // Завершаем программу с кодом ошибки
        }
    } while (result > 0);

    // Завершаем отправку данных на клиентский сокет
    result = shutdown(ClientSocket, SD_SEND);
    if (result == SOCKET_ERROR)
    {
        closesocket(ClientSocket); // Закрываем клиентский сокет
        freeaddrinfo(addResult); // Освобождаем результаты getaddrinfo
        WSACleanup(); // Чистим ресурсы Winsock
        return 1; // Завершаем программу с кодом ошибки
    }

    // Закрываем клиентский сокет и чистим ресурсы
    closesocket(ClientSocket);
    freeaddrinfo(addResult);
    WSACleanup();
    return 0;
}
