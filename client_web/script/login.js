const loginButton = document.getElementById('login');
const usernameInput = document.getElementById('username-field');
const passwordInput = document.getElementById('password-field');


function login(){
    const username = usernameInput.value;
    const password = passwordInput.value;

    const loginData = {
        type: "login",
        username: username,
        password: password
    };
    socket.send(JSON.stringify(loginData));
}

const response_array = [];

socket.onmessage = function(event){
    const response = JSON.parse(event.data);
    if (response["type"] === "login_failure"){
        alert("login failure. Reason: " + response["reason"]);
    }

    response_array.push(response);
    if (response_array.length === 3){
        sessionStorage.setItem("response_array", JSON.stringify(response_array));
        window.location.replace('home.html');
    }
}

if (loginButton){
    loginButton.addEventListener('click', login);
}
