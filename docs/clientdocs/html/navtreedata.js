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
  [ "msgClient", "index.html", [
    [ "Архитектура проекта: Чат-клиент (Версия 4)", "md_description.html", [
      [ "1. Общее видение и принципы", "md_description.html#autotoc_md1", [
        [ "Ключевые архитектурные принципы:", "md_description.html#autotoc_md2", null ]
      ] ],
      [ "2. Диаграмма архитектуры", "md_description.html#autotoc_md4", null ],
      [ "3. Компонентный разбор", "md_description.html#autotoc_md6", [
        [ "3.1. <span class=\"tt\">MainWindow</span>: Ядро приложения", "md_description.html#autotoc_md7", null ],
        [ "3.2. Компоненты чата (<span class=\"tt\">ChatViewWidget</span>, <span class=\"tt\">SmoothListView</span>, <span class=\"tt\">ChatMessageDelegate</span>, <span class=\"tt\">ChatMessageModel</span>)", "md_description.html#autotoc_md8", null ],
        [ "3.3. Компоненты списка контактов (<span class=\"tt\">QListWidget</span>, <span class=\"tt\">ContactListDelegate</span>)", "md_description.html#autotoc_md9", null ],
        [ "3.4. Структуры данных (<span class=\"tt\">structures.h</span>)", "md_description.html#autotoc_md10", null ]
      ] ],
      [ "4. Итоги по архитектуре V4", "md_description.html#autotoc_md12", null ]
    ] ],
    [ "Анализ изменений: Переход от Версии 5 к Версии 6", "md_update.html", [
      [ "1. 🏛️ Главное архитектурное изменение: <span class=\"tt\">QListWidget</span> → <span class=\"tt\">QListView</span> + <span class=\"tt\">QAbstractListModel</span>", "md_update.html#autotoc_md14", null ],
      [ "2. 🎨 Визуальные улучшения и полировка UX", "md_update.html#autotoc_md16", [
        [ "2.1. Векторные SVG-иконки для статусов сообщений", "md_update.html#autotoc_md17", null ]
      ] ],
      [ "3. 🧠 Улучшение логики и функциональности", "md_update.html#autotoc_md19", [
        [ "3.1. Более надежный механизм пометки сообщений как \"Прочитано\"", "md_update.html#autotoc_md20", null ]
      ] ],
      [ "Сводная таблица ключевых изменений", "md_update.html#autotoc_md21", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Macros", "globals_defs.html", null ]
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