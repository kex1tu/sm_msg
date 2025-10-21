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
        contact.textContent = item["displayname"];
        contact.classList.add("contact");
        contact.id = '@' + item["username"];
        contact.addEventListener('click', () => { //Добавляем прослушку каждой кнопке. Она будет делать нажатую кнопку выбранной, отправлять запрос на сервер, забирать статус выбранной у другой кнопки, а так же проверять, не нажали ли одну и ту же кнопку дважды
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

export const user_array_manager = function(array, user, delete_user = false){
    if (delete_user === true){
        for (i = 0; i < array.length(); ++i){
            if (array[i] === user){
                array.splice(i, 1); //Удаляем один элемент
                return;
            }
        }
    }
    if (!(user in array)){
        array.push(user);
    }
    
}
