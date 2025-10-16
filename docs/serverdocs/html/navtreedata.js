/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "msgServer", "index.html", [
    [ "Описание архитектуры и структуры проекта: Чат-сервер (Версия 4)", "md_description.html", [
      [ "1. Обзор проекта", "md_description.html#autotoc_md1", [
        [ "Ключевые технологии и парадигмы:", "md_description.html#autotoc_md2", null ]
      ] ],
      [ "2. Ключевые архитектурные решения", "md_description.html#autotoc_md4", [
        [ "2.1. Гибридный сетевой уровень (TCP + WebSocket)", "md_description.html#autotoc_md5", null ],
        [ "2.2. Обработка команд на основе карты обработчиков (Handler Map)", "md_description.html#autotoc_md6", null ],
        [ "2.3. Управление состоянием клиентов", "md_description.html#autotoc_md7", null ],
        [ "2.4. Слой персистентности (База данных SQLite)", "md_description.html#autotoc_md8", null ]
      ] ],
      [ "3. Компонентный разбор и жизненный цикл запроса", "md_description.html#autotoc_md10", null ],
      [ "4. Итоги по архитектуре", "md_description.html#autotoc_md12", null ]
    ] ],
    [ "Архитектурный рефакторинг: Переход на мультипротокольную модель (TCP + WebSocket)", "md_update.html", [
      [ "1. Мотивация для изменений", "md_update.html#autotoc_md14", null ],
      [ "2. Сравнение архитектур \"До\" и \"После\"", "md_update.html#autotoc_md15", null ],
      [ "3. Ключевые этапы перехода и изменения в коде", "md_update.html#autotoc_md18", [
        [ "Шаг 1: Изменение базового класса", "md_update.html#autotoc_md19", null ],
        [ "Шаг 2: Введение серверных объектов (Композиция)", "md_update.html#autotoc_md20", null ],
        [ "Шаг 3: Обновление логики запуска сервера", "md_update.html#autotoc_md21", null ],
        [ "Шаг 4: Новый механизм обработки подключений", "md_update.html#autotoc_md22", null ],
        [ "Шаг 5: Унификация управления клиентами", "md_update.html#autotoc_md23", null ],
        [ "Шаг 6: Абстрагирование отправки данных", "md_update.html#autotoc_md24", null ],
        [ "Шаг 7: Адаптация обработчиков (Handlers)", "md_update.html#autotoc_md25", null ]
      ] ],
      [ "4. Преимущества новой архитектуры", "md_update.html#autotoc_md26", null ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';