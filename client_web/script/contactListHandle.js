const cancel_message_hat = function(){
    const message_top_area = document.getElementById('message_top_area');
    const message_header_field = document.getElementById('message_header_field');
    const to_message = document.getElementById('to_message');
    message_header_field.textContent = '';
    to_message.textContent = '';
    message_top_area.classList.add('hidden');
}


export const update_html_contact_list = function(user_array){
    const contact_list_element = document.getElementById('contact_list'); //HTML элемент, представляющий список пользователей
    for (const item of user_array){
        if (document.getElementById('@' + item["username"])){
            continue;
        } //Если элемент уже есть, не добавляем его
        const contact = document.createElement('li');
        const request = {
            "type": "get_history",
            "with_user": item["username"]
        } //Формируем запрос для отправки через прослушку
        if (item['displayname'].length > 40){
            contact.textContent = item["displayname"].slice(0, 40);
        }
        else{
            contact.textContent = item["displayname"];
        }
        contact.classList.add("contact");
        contact.id = '@' + item["username"];
        contact.addEventListener('click', () => { //Добавляем прослушку каждой кнопке. Она будет делать нажатую кнопку выбранной, отправлять запрос на сервер, забирать статус выбранной у другой кнопки, а так же проверять, не нажали ли одну и ту же кнопку дважды
            cancel_message_hat();
            for (const elem of contact_list_element.children){ //Пробегаемся по всем контактам
                if (elem.classList.contains('current_contact')){
                    if (elem.id === contact.id){
                        return; //Если пользователь повторно запросит у сервера историю с уже выбранным пользователем, игнорируем запрос
                    }
                    else{
                        elem.classList.remove('current_contact'); //Нужно для отслеживания выбранного пользователя а так же для изменения стиля
                    }
                }
            }
            contact.classList.add('current_contact');
            socket.send(JSON.stringify(request));
        });
        contact_list_element.appendChild(contact);
    }
}

