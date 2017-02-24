$(document).ready(function(){

    allCheckboxSetup();
    requestLineCheckboxSetup();
    headersCheckboxSetup();
    eachCheckboxSetup();
    requestLineClassCheckboxSetup();
    headersClassCheckboxSetup();

    checkCheckBoxes();
    changeWidth();
});

function checkCheckBoxes() {
    var requestLineChecked = true;
    $('.requestLine').each(function(){
        if(!$(this).prop("checked")) {
            requestLineChecked = false;
            return false;
        }
    });
    $("#requestLineCheckbox").prop("checked", requestLineChecked);

    var headersChecked = true;
    $('.headers').each(function(){
        if(!$(this).prop("checked")) {
            headersChecked = false;
            return false;
        }
    });
    $("#headersCheckbox").prop("checked", headersChecked);

    if (requestLineChecked && headersChecked)
        $("#allCheckbox").prop("checked", headersChecked);
}

function allCheckboxSetup() {
    $('#allCheckbox').click(function(){
        $("input").prop("checked", $(this).prop("checked"));
    });
}

function requestLineCheckboxSetup() {
    $('#requestLineCheckbox').click(function(){
        if ($(this).prop("checked")) {
            $(".requestLine").prop("checked", true);
            if ($('#headersCheckbox').prop("checked"))
                $("#allCheckbox").prop("checked", true);
        } else {
            $(".requestLine").prop("checked", false);
        }
    });
}

function headersCheckboxSetup() {
    $('#headersCheckbox').click(function(){
        if ($(this).prop("checked")) {
            $(".headers").prop("checked", true);
            if ($('#requestLineCheckbox').prop("checked"))
                $("#allCheckbox").prop("checked", true);
        } else {
            $(".headers").prop("checked", false);
        }
    });
}

function eachCheckboxSetup() {
    $('input[type="checkbox"]').click(function(){
        if (!$(this).prop("checked"))
            $("#allCheckbox").prop("checked", false);
    });
}

function requestLineClassCheckboxSetup() {
    $('.requestLine').click(function(){
        if (!$(this).prop("checked")) {
            $("#requestLineCheckbox").prop("checked", false);
        } else {
            var allChecked = true;
            $('.requestLine').each(function(){
                if(!$(this).prop("checked")) {
                    allChecked = false;
                    return false;
                }
            });
            if (allChecked) {
                $("#requestLineCheckbox").prop("checked", true);
                if ($('#headersCheckbox').prop("checked"))
                    $("#allCheckbox").prop("checked", true);
            }
        }
    });
}

function headersClassCheckboxSetup() {
    $('.headers').click(function(){
        if (!$(this).prop("checked")) {
            $("#headersCheckbox").prop("checked", false);
        } else {
            var allChecked = true;
            $('.headers').each(function(){
                if(!$(this).prop("checked")) {
                    allChecked = false;
                    return false;
                }
            });
            if (allChecked) {
                $("#headersCheckbox").prop("checked", true);
                if ($('#requestLineCheckbox').prop("checked"))
                    $("#allCheckbox").prop("checked", true);
            }
        }
    });
}

function changeWidth() {
    var table = document.getElementsByTagName('table')[0];
    var section = document.getElementsByTagName('section')[0];
    var article = document.getElementsByTagName('article')[0];
    var main = document.getElementsByTagName('main')[0];
    var aside = document.getElementsByTagName('aside')[0];

    var articleSidePadding = parseInt($('article').css('padding-left')) + parseInt($('article').css('padding-right'));
    if ((table.offsetWidth + articleSidePadding) > section.offsetWidth) {
        article.style.width = table.offsetWidth + 'px';
        section.style.width = article.offsetWidth + 'px';
        var marginInsideOfMain = parseInt($('aside').css('margin-left')) + parseInt($('section').css('margin-left'));
        main.style.width = marginInsideOfMain + section.offsetWidth + aside.offsetWidth + 'px';
    }
}
