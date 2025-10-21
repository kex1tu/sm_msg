import * as contactListHandle from './contactListHandle.js';
import * as messageHandle from './messageHandle.js';
import * as privateMessageHandle from './privateMessageHandle.js';

//--------ЭЛЕМЕНТЫ HTML СТРАНИЦЫ-----------

const loginButton = document.getElementById('login');
const usernameInput = document.getElementById('username_field');
const passwordInput = document.getElementById('password_field');
const loginWrapper = document.getElementById('Login_wrapper');
const mainWrapper = document.getElementById('Main_wrapper');
const message_list_html = document.getElementById('message_list');
const contact_list_html = document.getElementById('contact_list');
const right_column = document.getElementById('right_column');

//----------------ПЕРЕМЕННЫЕ С ОБЩЕЙ ОБЛАСТЬЮ ВИДИМОСТИ-----------------

let current_chat_username; //Отслеживаем, с каким пользователем открыт чат
let first_message_in_chat; //ID первого в списке сообщения в чате
let previous_first_message; //Запоминаем переменную строчкой выше, когда происходит вызов функции на прокрутке
const user_array = []; //Массив с объектами user = 
// {"username": "123", 
// "displayname": "123", 
// "last_seen": ""}

// class User{
//     constructor(username, displayname, last_seen){
//         this.username = username;
//         this.displayname = displayname;
//         this.last_seen = last_seen;
//     }
// }

//-----------ФУНКЦИИ-ПРОСЛУШКИ-----------

function login(){
    const username = usernameInput.value;
    const password = passwordInput.value;

    const loginData = {
        type: "login",
        username: username,
        password: password
    };
    sessionStorage.setItem('my_username', username);
    socket.send(JSON.stringify(loginData));
}
if (loginButton){
    loginButton.addEventListener('click', login);
}


message_list_html.addEventListener('scroll', () => { //Обработка прокручивания списка сообщений
    if (previous_first_message === first_message_in_chat){return;} //Не отправляем запрос, если достигли начала истории или если происходит слишком частый вызов функции
    if (message_list_html.scrollTop <= 20){
        const request = {
            "type": "get_history",
            "with_user": current_chat_username,
            "before_id": first_message_in_chat
        }
        socket.send(JSON.stringify(request));
        previous_first_message = first_message_in_chat;
    }
})

//----------ЕДИНСТВЕННАЯ ПРОСЛУШКА СОКЕТА----------

socket.onmessage = function(event){
    let response = JSON.parse(event.data);
    let tmp_array = [];
    
    switch (response["type"]){
        case "login_success":
            alert("Выполнен вход");
            loginWrapper.classList.add("hidden");
            mainWrapper.classList.remove("hidden");
            break;
        case "login_failure":
            alert("login failure. Reason: " + response["reason"]);
            break;
        case "contact_list":
            tmp_array = response["users"];
            tmp_array.forEach(element => {
                contactListHandle.user_array_manager(user_array, element);
            });
            contactListHandle.update_html_contact_list(user_array);
            break;
        case "history_data":
            current_chat_username = response["with_user"];
            tmp_array = response["history"];
            messageHandle.update_html_message_list(tmp_array, true);
            first_message_in_chat = tmp_array[0]['id'];
            message_list_html.scrollTo(0, message_list_html.scrollHeight);
            break;
        case "old_history_data":
            tmp_array = response["history"];
            messageHandle.update_html_message_list(tmp_array, false);
            if (tmp_array.length !== 0){
                first_message_in_chat = tmp_array[0]['id'];
            }
            break;
    }
}