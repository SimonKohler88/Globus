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



function get_motor_target_speed() {
    // (B1) GET HTML FORM DATA
    let form = new FormData(document.getElementById("id_mot_ctrl_form"));

    // (B2) SUBMIT FORM VIA POST
    fetch(`motor_target_speed`, {
        method: "GET",
        // body: form
    })
        .then(res => res.text())
        .then(txt => {
            // DO SOMETHING ON SERVER RESPONSE
            console.log(txt);
            // DO SOMETHING ON SERVER RESPONSE
            document.getElementById("mySlider").value = parseInt(txt);
            document.getElementById("sliderValueBg").innerText = txt;
            // document.getElementById("curr_speed").innerHTML = txt;
        })
        .catch(e => console.error(e));

    // (B3) STOP FORM SUBMIT
    return false;
}

window.onload = () => {
    //let's go
    console.log("doc ready");
    get_motor_target_speed();

    // Slider value background logic
    const slider = document.getElementById("mySlider");
    const sliderValueBg = document.getElementById("sliderValueBg");
    if (slider && sliderValueBg) {
        slider.addEventListener("input", () => {
            sliderValueBg.innerText = slider.value;
        });
    }


    let intervalId = setInterval(function () {
        // alert("Interval reached every 5s")
        fetch(`motor_speed`, {
            method: "GET",
            // body: form
        })
            .then(res => res.text())
            .then(txt => {
                // DO SOMETHING ON SERVER RESPONSE
                // console.log(txt);
                document.getElementById("speed_meter").value = parseInt(txt);
                document.getElementById("curr_speed").innerText = txt;

            })
            .catch(e => console.error(e));

        // (B3) STOP FORM SUBMIT
        return false;
    }, 1000);
};

