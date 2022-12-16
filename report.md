# 4 балла.

## Условие задачи с пояснениями вывода и связями с кодом
Магазин состоит из трех отделов (`Department`), в каждом из которых есть один продавец.  
Покупатель (`Customer`), заходя в магазин, выбирает разделы, которые хочет посетить (они могут повторяться).  
Тогда покупатель говорит: "Started shopping, want to visit \<dep_count\>", где \<dep_count\> - количество отделов, которые он хочет посетить.  
Покупатель, приходя в очередной отдел со словами: "Going to department #\<department_id\>", встает в очередь к отделу и "засыпает", пока не придет его очередь.  
Как только приходит очередь покупателя, продавец начинает обслуживать его и говорит: "Serving customer #\<customer_id>\", где \<customer_id\> - идентификатор покупателя.  
Уходя из отдела, продавец говорит: "Returning from department #\<customer_id>\".
После того как покупатель обслужен, продавец говорит: "Finished serving customer #\<customer_id>\".  
Покупатель, после того как посетил все отделы, говорит: "Finished shopping".  
Программа завершает работу после того как все покупатели посетят свои отделы.

## Модель параллельных вычислений
Была выбрана модель "клиент-сервер".  
На каждого покупателя создается отдельный поток (`thread`).  
Далее в функции `startShopping` (которая передается в `pthread_create`) покупатель начинает проходится по отделам, которые он хочет посетить.  
Приходя в очередной отдел (`serveCustomer`), продавец начинает его обслуживать. Само обслуживание находится внутри мьютекса (`department_mutex`), который уникален для каждого экземпляра отдела, чтобы не было одновременного обслуживания нескольких покупателей. Таким образом, потоки покупателей блокируется, пока не придет его очередь. После того как поток-покупатель был обслужен продавцом, он освобождает очередь - открывает мьютекс - что позволяет следующему потоку взаимодействовать с продавцом. Логично, что только тот, кто первый в очереди, может это место и освободить (то есть тот, кто мьютекс заблокировал, тот его и откроет, в отличие от семафора, где открыть его может любой поток). Также используется `cout_mutex` для блокировки вывода в консоль, чтобы не было смешивания вывода разных потоков. Условие задачи наиболее подходит модели "клиент-сервер" (или "клиенты и серверы"), с серверами, которые обрабатывают запросы клиентов последовательно. Имеем 3 сервера (3 отдела магазина), клиенты (покупатели) делают запрос одному из серверов и ожидают ответа, прежде чем сделать запрос к другому серверу. Запрос клиента попадает в очередь, сервер последовательно обрабатывает эту очередь запросов. При этом, в данной реализации очередь запросов клиентов у сервера теоретически может иметь сколь угодно большой размер.  
Источник информации с данной моделью: https://pro-prof.com/forums/topic/parallel-programming-paradigms  

## Входные данные: формат ввода отделов магазина
Отделы магазина задаются в формате: \<department_id1\>\<department_id2\> ... \<department_idn\>, где \<department_id\> - целое число, обозначающее id отдела.  
Всего есть только три отдела: 1, 2, 3, поэтому должно выполняться следующее утверждение: 1 <= \<department_id1\> <= 3.  
Примеры ввода отделов магазина:
- 123 \[прийти в 1-ый, потом во 2-ой, потом в 3-ий\]
- 31 \[прийти в 3-ий, потом в 1-ый\]
- 2 \[прийти только во 2-ой\]
- 1221312321 \[надеюсь, понятно\]

## Ввод данных: количество покупателей
Количество покупателей задается положительным целым числом.  
Следует понимать, что модель предполагает, что на каждого покупателя будет создан отдельный поток.  
*Автор не рекомендует вводить большие числа для создания потоков, программа может вылететь но это зависит от компьютера, поэтому четкого порога нет*

## Ввод данных (через консоль)
Для ввода данных через консоль необходимо запустить программу без аргументов (т. е. просто ввести `./prog`).  
Программа подскажет пользователю, что выбран способ ввода через консоль.  
Сначала программа попросит ввести количество покупателей.  
Далее для каждого пользователя нужно ввести отделы магазина, в которые ему надо будет посетить. (см. пункт про формат ввода отделов магазина).  

## Ввод данных (через аргументы командной строки)
Достаточно указать аргументы командной строки при запуске программы.  
Программа подскажет пользователю, что выбран способ ввода через аргументы командной строки.  
Отделы магазина для каждого покупателя записываются вместе, без пробелов (см. пункт про формат ввода отделов магазина).  
Каждый такой "набор" отделов покупателя отделяется пробелом пробелами.  
Количество покупателей определяется автоматически.  
Пример запуска программы с аргументами командной строки: 
- `./prog 123 31 2 1221312321` (4 покупателя).
- `./prog 1 2 3` (3 покупателя).
- `./prog 123` (1 покупатель).

## Ввод данных: некорректный входные данные
Программа постарается поймать некорректные входные данные.  
Будет выведена ошибка с пояснением и программа завершится (не аварийно).   

## Результаты работы программы
(очевидно, что результаты работы программы могут отличаться от приведенных ниже - это многопоточное приложение)  

1\.

    $ ./prog 1 2 3
    (using command line args for customers)
    [Customer #1] Started shopping, want to visit 1 departments
    [Customer #1] Going to department #1
    [Department #1] Serving customer #1
    [Customer #3] Started shopping, want to visit 1 departments
    [Customer #3] Going to department #3
    [Department #3] Serving customer #3
    [Customer #2] Started shopping, want to visit 1 departments
    [Customer #2] Going to department #2
    [Department #2] Serving customer #2
    [Department #1] Finished serving customer #1
    [Customer #1] Returning from department #1
    [Customer #1] Finished shopping
    [Department #3] Finished serving customer #3
    [Customer #3] Returning from department #3
    [Customer #3] Finished shopping
    [Department #2] Finished serving customer #2
    [Customer #2] Returning from department #2
    [Customer #2] Finished shopping

2\. 

    $ ./prog
    (using console input for customers)
    Enter the number of customers: 4
    Enter the departments for customer 1: 1
    Enter the departments for customer 2: 23 
    Enter the departments for customer 3: 123
    Enter the departments for customer 4: 2
    [Customer #1] Started shopping, want to visit 1 departments
    [Customer #1] Going to department #1
    [Department #1] Serving customer #1
    [Customer #2] Started shopping, want to visit 2 departments
    [Customer #3] Started shopping, want to visit 3 departments
    [Customer #4] Started shopping, want to visit 1 departments
    [Customer #4] Going to department #2
    [Department #2] Serving customer #4
    [Customer #3] Going to department #1
    [Customer #2] Going to department #2
    [Department #1] Finished serving customer #1
    [Customer #1] Returning from department #1
    [Customer #1] Finished shopping
    [Department #1] Serving customer #3
    [Department #2] Finished serving customer #4
    [Customer #4] Returning from department #2
    [Customer #4] Finished shopping
    [Department #2] Serving customer #2
    [Department #1] Finished serving customer #3
    [Customer #3] Returning from department #1
    [Customer #3] Going to department #2
    [Department #2] Finished serving customer #2
    [Customer #2] Returning from department #2
    [Customer #2] Going to department #3
    [Department #3] Serving customer #2
    [Department #2] Serving customer #3
    [Department #3] Finished serving customer #2
    [Customer #2] Returning from department #3
    [Customer #2] Finished shopping
    [Department #2] Finished serving customer #3
    [Customer #3] Returning from department #2
    [Customer #3] Going to department #3
    [Department #3] Serving customer #3
    [Department #3] Finished serving customer #3
    [Customer #3] Returning from department #3
    [Customer #3] Finished shopping

3\.

    $ ./prog 1 1
    (using command line args for customers)
    [Customer #1] Started shopping, want to visit 1 departments
    [Customer #2] Started shopping, want to visit 1 departments
    [Customer #2] Going to department #1
    [Department #1] Serving customer #2
    [Customer #1] Going to department #1
    [Department #1] Finished serving customer #2
    [Customer #2] Returning from department #1
    [Customer #2] Finished shopping
    [Department #1] Serving customer #1
    [Department #1] Finished serving customer #1
    [Customer #1] Returning from department #1
    [Customer #1] Finished shopping

# 5 баллов.
(Комментарии см. в исходном коде программы.)

Сценарий поведения программы.  
Клиент (покупатель):
- Отправляет запросы серверу на обслуживание.
- Ждет ответа (окончания обработки запросы) от сервера.
- Только после этого, отправляет следующий запрос.
  
Сервер (отдел магазина):
- Принимает запросы от клиентов, добавляет их в очередь.
- Обслуживает первый в очереди запрос и перемещает очередь.
- Отправляет ответ клиенту (заканчивает обработку запроса), который отправил запрос.

Пример возможного сценария представлен выше в "Результаты работы программы".  
Прочие комментарии см. в "Модель параллельных вычислений".  

# 6 баллов.
Алгоритм исходного словесного сценария.  
1. Создать клиентов (покупателей) и серверы (3 отдела магазина) через входные данные.
2. Создать отдельный поток для каждого клиента.
3. Пока у клиента не пуста очередь запросов, отправить запрос соответствующему серверу.
    - Сервер добавляет запрос в очередь запросов.
    - Клиент ждет ответа на запрос у сервера ("спит").
    - Сервер обрабатывает запросы в очереди, отправляя ответы клиентам, доходя до запроса данного клиента.
    - Данный клиент просыпается и продолжает обработку очереди запросов.
4. Клиент завершает работу при выполнении всех запросов.
5. Программа продолжает работать пока хоть один из клиентов имеет неисполненные запросы.
6. После этого программа завершает работу.

(технические особенности реализации см. в "Модель параллельных вычислений" и в коде)  

Ввод данных из командной строки реализован. Формат ввода описан выше.  