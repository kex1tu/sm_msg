export const update_html_message_list = function(message_array, new_user = false){
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
        
        //TODO: Тут еще херова гора параметров, которые понадобятся позже.
    }
}