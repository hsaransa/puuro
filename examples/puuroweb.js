var server = false;
var server_pos = 0;

function makeRequest(url, fnc) {
    var req = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        req = new XMLHttpRequest();
        if (req.overrideMimeType) {
            req.overrideMimeType('text/plain');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            req = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
                req = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!req) {
        alert('Cannot create XMLHTTP instance');
        return false;
    }

    req.onreadystatechange = fnc;
    req.open('POST', url, true);

    return req;
}

function server_ready_change() {
    elem = document.getElementById("debug");
    if (elem != null && server.readyState >= 1)
        elem.innerHTML = server.status + " " + server.readyState + "<br>";

    if (server.readyState < 3)
        return;

    if (server.status != 200)
        return;

    var s = server.responseText.substring(server_pos);

    if (s == null)
        return;

    i = 0;
    while (s.indexOf(';', i) != -1)
    {
        cmd = s.substr(i, s.indexOf(';', i) - i);
        i += cmd.length + 1;

        if (cmd == "view")
        {
            view_num = parseInt(s.substr(i, s.indexOf(';', i) - i));
            i = s.indexOf(';', i) + 1;
        }
        else if (cmd == "set")
        {
            len = parseInt(s.substr(i, s.indexOf(';', i) - i));
            i = s.indexOf(';', i) + 1;

            data = s.substr(i, len);
            i += len;

            elem = document.getElementById("content");
            elem.innerHTML = data;

            if (s.charAt(i) != ';')
            {
                alert("Error, expecting semi-color: " + s.substr(i));
                return;
            }

            i++;
        }
        else if (cmd == "new")
        {
            len = parseInt(s.substr(i, s.indexOf(';', i) - i));
            
            i = s.indexOf(';', i) + 1;

            data = s.substr(i, len);
            i += len;

            elem = document.getElementById("msgs");
            elem.innerHTML = elem.innerHTML + data + "<br>";

/*
            if (s.charAt(i) != ';')
            {
                alert("Error, expecting semi-color: " + s.substr(i));
                return;
            }

            i++;
            */
        }
        else
        {
            alert("Error, bad cmd: " + cmd);
        }
    }

    server_pos += i;

    if (server.readyState == 4) {
        server_pos = 0;
        server = false;
    }
}


/* Sending data */

var send_buffer = "";
var send_request = false;

function client_ready_change()
{
    if (send_request.readyState == 4) {
        send_request = false;
        send_data("");
    }
}

function send_data()
{
    if (send_request)
    {
        setTimeout("send_data(\"\");", 500);
        return;
    }

    if (send_buffer != "")
    {
        req = makeRequest("act?" + view_num, client_ready_change);
        var plop = send_buffer;
        send_buffer = "";
        req.send(plop + "end;");
    }
}

var trying_to_send = false;

function try_to_send()
{
    send_data();
    trying_to_send = false;
}

function send_command(s)
{
    send_buffer += s + ";";

    if (!trying_to_send)
    {
        trying_to_send = true;
        setTimeout("try_to_send();", 100);
    }
}

/* This function updates element value to server. */

function send_value(ev, elem)
{
    send_command("set;" + elem.id + ";" + elem.value.length + ";" + elem.value);
}

/* Make server connection so that it can push data. */

function on_load()
{
    elem = document.getElementById("debug");
    if (elem != null)
        elem.innerHTML = "hello from on_load()<br>";

    setTimeout("wake_up_server();", 100);
}

function wake_up_server()
{
    if (server == false)
    {
        server = makeRequest("push?" + view_num, server_ready_change);
        server.send("Lollero");
    }
    setTimeout("wake_up_server();", 100);
}

function send()
{
    elem = document.getElementById("field");
    send_command("msg;" + elem.innerHTML);
}
