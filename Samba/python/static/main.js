//let /const, kein var!!


function submit_motor_control(command) {
    // (B1) GET HTML FORM DATA
    let form = new FormData(document.getElementById("id_mot_ctrl_form"));

    // (B2) SUBMIT FORM VIA POST
    fetch(`motor_control/${command}`, {
        method: "GET",
        // body: form
    })
        .then(res => res.text())
        .then(txt => {
            // DO SOMETHING ON SERVER RESPONSE
            console.log(txt);
        })
        .catch(e => console.error(e));

    // (B3) STOP FORM SUBMIT
    return false;
}



function submit_jpeg() {
    // (B1) GET HTML FORM DATA
    let form = new FormData(document.getElementById("id_jpeg_upload_form"));

    // (B2) SUBMIT FORM VIA POST
    fetch("upload", {
        method: "POST",
        body: form
    })
        .then(res => res.text())
        .then(txt => {
            // DO SOMETHING ON SERVER RESPONSE
            console.log(txt);
        })
        .catch(e => console.error(e));

    // (B3) STOP FORM SUBMIT
    return false;
}


function submit_globe_view() {
    // (B1) GET HTML FORM DATA
    let form = new FormData(document.getElementById("id_globe_selector"));

    // (B2) SUBMIT FORM VIA POST
    fetch("show_globe_video", {
        method: "POST",
        body: form
    })
        .then(res => res.text())
        .then(txt => {
            // DO SOMETHING ON SERVER RESPONSE
            console.log(txt);
        })
        .catch(e => console.error(e));

    // (B3) STOP FORM SUBMIT
    return false;
}

