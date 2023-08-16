const form = document.querySelector('#timingData');
if (form) {
    form.addEventListener("submit", function (e) {
        console.log('working');
        submitForm(e, this);
    });
}

async function submitForm(e, form) {
    e.preventDefault();

    const btnSubmit = document.getElementById('btnSubmit');
    btnSubmit.disabled = true;
    setTimeout(() => btnSubmit.disabled = false, 2000);

    const jsonFormData = buildJsonFormData(form);
    const headers = buildHeaders();

    const response = fetchService.performPostHttpRequest('/timings', headers, jsonFormData);
    console.log(response);

    if (response)
        alert('Changes Saved');
    else
        alert('an error occured');
}

function buildJsonFormData(form) {
    const jsonFormData = {};
    for (const pair of new FormData(form)) {
        jsonFormData[pair[0]] = pair[1];
    }
    return jsonFormData;
}

async function performPostHttpRequest(fetchLink, headers, body) {
    if (!fetchLink || !headers || !body) {
        throw new Error("one or more parameters were not passed")
    }
    try {
        const rawResponse = await fetch(fetchLink, { methood: "POST", headers: headers, body: JSON.stringify(body) });


        const content = await rawResponse.json();
        return content;
    }
    catch (err) {
        console.error(`Error at fetch POST: ${err}`);
        throw err;
    }
}