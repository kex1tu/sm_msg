const registerButton = document.getElementById('register');
const responseMessage = document.getElementById('response-message');
const displaynameInput = document.getElementById('displayname-field');
const usernameInput = document.getElementById('username-field');
const passwordInput = document.getElementById('password-field');

function register(){
    const displayname = displaynameInput.value;
    const username = usernameInput.value;
    const password = passwordInput.value;

    const registerData = {
        type: "register",
        display_name: displayname,
        username: username,
        password: password
    };
    socket.send(JSON.stringify(registerData));
}

socket.onmessage = function(event){
    
    const response = JSON.parse(event.data);

    if (response["type"] === "register_success"){
        alert("Регистрация успешна!");
    } 
    else{
        if (response["type"] === "register_failure" && response["reason"] === "Username already exists."){
            alert("Пользователь уже существует");
        }
        else{
            alert("Неизвестная ошибка. Причина: ", response["reason"]);
        }
    }
    
}

registerButton.addEventListener('click', register);