export const message_history_load_html = function(message_array, new_user = false){
    const message_list_element = document.getElementById('message_list');
    if (new_user === true){ //Удаляем все элементы из контейнера для сообщений на странице
        while(message_list_element.firstChild){
            message_list_element.removeChild(message_list_element.firstChild);
        }
    }

    for (const item of new_user === true? message_array : message_array.slice().reverse()){
        const message = document.createElement('p');
        message.textContent = item["payload"];
        message.id = '#' + item["id"];
        message.classList.add('message');
        if (item["fromUser"] === sessionStorage.getItem('my_username')){
            message.classList.add('my_message');
        }
        if (new_user === false){
            message_list_element.prepend(message); 
        }
        else{
            message_list_element.appendChild(message); 
        }
        
        //TODO: Тут еще много параметров, которые понадобятся позже.
    }
}

export const add_message_html = function(message){
    const message_list_html = document.getElementById('message_list');
    const message_html = document.createElement('p');
    message_html.textContent = message["payload"];
    message_html.id = '#' + message["id"];
    message_html.classList.add('message');
    message_list_html.appendChild(message_html);
}