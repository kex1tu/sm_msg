export const update_html_search_database = function(array){
    const contact_search_database = document.getElementById('contact_search_datalist');
    while (contact_search_database.firstChild){
        contact_search_database.removeChild(contact_search_database.firstChild);
    }
    for (const item of array){
        const option = document.createElement('option');
        option.value = item['username'];
        contact_search_database.appendChild(option);
    }
}