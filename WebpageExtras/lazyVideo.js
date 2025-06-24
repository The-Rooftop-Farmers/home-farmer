// Lazy load the video only when the placeholder is clicked
document.addEventListener('DOMContentLoaded', function() {
    const videoContainer = document.getElementById('video-container');
    const placeholder = document.getElementById('video-placeholder');

    if (placeholder && videoContainer) {
        placeholder.addEventListener('click', function() {
            placeholder.style.display = 'none';

            const video = document.createElement('video');
            video.src = "WebpageExtras/media/Project Video.mp4";
            video.controls = true;
            video.autoplay = true;
            video.style.width = '100%';
            video.style.height = '100%';
            video.style.position = 'absolute';
            video.style.top = 0;
            video.style.left = 0;
            video.style.borderRadius = '8px';
            videoContainer.appendChild(video);
            video.focus();
        });
    }
});