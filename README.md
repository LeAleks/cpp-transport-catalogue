# CityRouter
Программная реализация городсок системы маршрутизации, работающая с JSON запросами. Программа двухстадийная и имеет два варианта запросов: первый отвечает за создание базы данных, второй же выполняет всю нужную работу для того, чтобы выдать нужную информацию по любым оптимальным маршрутам, автобусам или остановкам. Форматы вывода: .json и .svg.

# Использование
0. Установка и настройка всех требуемых компонентов в среде разработки для запуска приложения
1. Вариант использования показан в main.cpp и examples.txt
2. "make_base": запрос на создание базы данных транспортного каталога
3. "process_request": запрос на получение любой информации по остановкам, автобусам, оптимальным маршрутам

# Системные требования
1. C++20 (STL)
2. GCC (MinGW-w64) 11.2.0

# Планы по доработке
1. Добавить карту города со спутника
2. Протестировать реальные координаты и остановки
3. Создать десктопное приложение

# Стек технологий
1. CMake 3.22.0
2. Protobuf-cpp 3.18.1
