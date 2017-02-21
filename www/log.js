$(document).ready(function(){
    allCheckboxSetup();
    requestLineCheckboxSetup();
    headersCheckboxSetup();
    eachCheckboxSetup();
    requestLineClassCheckboxSetup();
    headersClassCheckboxSetup();

    changeWidth();
});

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
    var main = document.getElementsByTagName('main')[0];
    var aside = document.getElementsByTagName('aside')[0];
    var article = document.getElementsByTagName('article')[0];
    var table = document.getElementsByTagName('table')[0];

    var articlePadding = parseInt(getComputedStyle(article, null).getPropertyValue('padding'));
    main.style.width = articlePadding * 2 + table.offsetWidth + 'px';

    var mainMarginLeft = parseInt(getComputedStyle(main, null).getPropertyValue('margin-left'));
    document.body.style.width = mainMarginLeft + aside.offsetWidth + main.offsetWidth + "px";
}
