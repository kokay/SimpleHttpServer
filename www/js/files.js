var currentPath = window.location.pathname;

$(document).ready(function(){
    var file = $('input[type="file"]');
    file.change(showProgress);
});

function showProgress() {
    var progressBox = $('#progressBox');
    if (this.files.length == 0) {
        progressBox.fadeOut();
        $('p.error').hide();
        return;
    }
    if (this.files.length > 10) {
        progressBox.fadeOut();
        $('p.error').show();
        return;
    }

    $('p.error').hide();

    progressBox.empty();
    progressBox.append('<h4>Upload Files</h4>');
    
    for (var i = 0; i < this.files.length; ++i) {
        progressBox.append("<p class='fileName'>" + this.files[i].name + "</p>");
        progressBox.append("<progress id='progress" + i + "' value='0' max='100'></progress>");
    }
    progressBox.append('<input type="button" value="Upload Files">');
    progressBox.fadeIn();
    
    $('input[type="button"]').click(uploadStart);
}

function uploadStart() {
    var i = 0;
    var files = document.getElementById('file');
    uploadFile(files, i);
}

function uploadFile(files, i) {
    var data = new FormData();
    data.append('file', files.files[i]);

    var request = new XMLHttpRequest();
    request.onreadystatechange = function () {
	if (this.readyState == 4 && this.status == 200) {
	    i++;
	    if (i < files.files.length) {
		uploadFile(files, i);
	    } else {
		$('table').html(this.responseText);
		$('input[type="button"]').prop('disabled', false)
		    .val("Done").click(function () {
			$('#progressBox').fadeOut();
			$('#file').val('');
		    });
	    }
		
	}
    }
    request.onprogress = function(event) {
	$('#progress' + i).val(Math.ceil(event.loaded/event.total) * 100);
    }
    request.open('POST', currentPath);
    request.send(data);
}
