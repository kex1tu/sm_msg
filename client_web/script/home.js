import * as contactListHandle from './contactListHandle.js';
import * as messageHandle from './messageHandle.js';
import * as privateMessageHandle from './privateMessageHandle.js';
import * as pendingContactHandle from './pendingContactHandle.js';
import * as contactSearchHandle from './contactSearchHandle.js';

//--------ЭЛЕМЕНТЫ HTML СТРАНИЦЫ-----------

const loginButton = document.getElementById('login');
const usernameInput = document.getElementById('username_field');
const passwordInput = document.getElementById('password_field');
const loginWrapper = document.getElementById('Login_wrapper');
const mainWrapper = document.getElementById('Main_wrapper');
const message_list_html = document.getElementById('message_list');
const contact_list_html = document.getElementById('contact_list');
const contact_search_html = document.getElementById('contact_search');
// const right_column = document.getElementById('right_column');
const send_button_html = document.getElementById('send_button');
const message_input_html = document.getElementById('message_input');
const pending_contact_list_html = document.getElementById('pending_contact_list');
const pending_contact_button_html = document.getElementById('pending_contact_button');
const pending_contact_image_html = document.getElementById('pending_contact_img');
const chat_img_html = document.getElementById('chat_image');

//----------------ПЕРЕМЕННЫЕ С ОБЩЕЙ ОБЛАСТЬЮ ВИДИМОСТИ-----------------

let current_chat_username; //Отслеживаем, с каким пользователем открыт чат
let first_message_in_chat; //ID первого в списке сообщения в чате
let previous_first_message; //Запоминаем переменную строчкой выше, когда происходит вызов функции на прокрутке
let temp_id_counter = 0;
const private_message_queue = []; //Очередь из личных сообщений с временными ID. Будем убирать сообщения отсюда, если сервер подтвердит отправку
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
    if (message_list_html.scrollTop <= 50){
        const request = {
            "type": "get_history",
            "with_user": current_chat_username,
            "before_id": first_message_in_chat
        }
        socket.send(JSON.stringify(request));
        previous_first_message = first_message_in_chat;
    }
})

send_button_html.addEventListener('click', () => {
    if (current_chat_username === undefined){return;}
    if (message_input_html.value === ''){return;}
    const temp_id = '#temp' + temp_id_counter.toString();
    ++temp_id_counter;

    const request = {
        "type": "private_message",
        "fromUser": sessionStorage.getItem('my_username'),
        "toUser": current_chat_username,
        "payload": message_input_html.value,
        "reply_to_id": '0',
        "temp_id": temp_id
    }

    socket.send(JSON.stringify(request));
    private_message_queue.push(temp_id);

    const message = document.createElement('p');
    message.textContent = request['payload'];
    message.id = temp_id;
    message.classList.add('message', 'my_message');
    message_list_html.appendChild(message);
    message_list_html.scrollTo(0, message_list_html.scrollHeight);
    message_input_html.value = '';
})

pending_contact_button_html.addEventListener('click', () => { //Свап списков чатов и запросов на переписку
    if (contact_list_html.classList.contains('hidden')){
        contact_list_html.classList.remove('hidden');
        pending_contact_list_html.classList.add('hidden');
        pending_contact_image_html.classList.remove('hidden');
        chat_img_html.classList.add('hidden');
    }
    else{
        contact_list_html.classList.add('hidden');
        pending_contact_list_html.classList.remove('hidden');
        pending_contact_image_html.classList.add('hidden');
        chat_img_html.classList.remove('hidden');
    }
})

contact_search_html.addEventListener('input', () => {
    const request = {
        'type': "search_users",
        'term': contact_search_html.value
    }
    socket.send(JSON.stringify(request));
})

contact_search_html.addEventListener('keydown', function(event){
    if (event.key === 'Enter'){
        const request = {
            'type': 'add_contact_request',
            'username': contact_search_html.value
        }
        socket.send(JSON.stringify(request));
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
                // contactListHandle.user_array_manager(user_array, element);
                user_array.push(element);
            });
            contactListHandle.update_html_contact_list(tmp_array);
            break;
        case "history_data":
            current_chat_username = response["with_user"];
            tmp_array = response["history"];
            messageHandle.message_history_load_html(tmp_array, true);
            if (tmp_array.length !== 0){
                first_message_in_chat = tmp_array[0]['id'];
            }

            // last_message_id = message_list_html.lastChild.id;
            message_list_html.scrollTo(0, message_list_html.scrollHeight);
            break;
        case "old_history_data":
            tmp_array = response["history"];
            messageHandle.message_history_load_html(tmp_array, false);
            if (tmp_array.length !== 0){
                first_message_in_chat = tmp_array[0]['id'];
            }
            break;
        case "private_message":
            if (response['temp_id']){ //Наличие поля temp_id говорит о том, что сервер отправил эхо нашего же сообщения
                let temp_id = response['temp_id'];
                if (!(private_message_queue.includes(temp_id))){ //Если такого сообщения мы не отправляли, игнорируем его
                    break;
                }
                private_message_queue.splice(private_message_queue.indexOf(temp_id), 1); //Удаляем временный ID из очереди, так как сервер отчитался о его доставке
                const message_element = document.getElementById(temp_id);
                message_element.id = response['id'];
            }
            else{
                if (current_chat_username === response['fromUser']){
                    messageHandle.add_message_html(response);
                    message_list_html.scrollTo(0, message_list_html.scrollHeight);
                }
            }
            break;
        case "pending_requests_list":
            tmp_array = response['requests'];
            pendingContactHandle.pending_contact_list_load_html(tmp_array);
            break;
        case "incoming_contact_request":
            tmp_array = [];
            tmp_array.push(response);
            pendingContactHandle.pending_contact_list_load_html(tmp_array);
            break;
        case "search_results":
            tmp_array = response['users'];
            contactSearchHandle.update_html_search_database(tmp_array);
            break;
        case "add_contact_success":
            alert(response['reason']);
            contact_search_html.value = '';
            break;
        case "add_contact_failure":
            alert(response['reason']);
            break;
    }
}