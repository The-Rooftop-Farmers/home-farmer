// Smooth scrolling for all nav links in the webpage (Disables external navigation links in navbar)
document.addEventListener('DOMContentLoaded', function() {
    const navLinks = document.getElementsByClassName('internal-nav');
    const headerHeight = document.querySelector('header').offsetHeight;
    const navHeight = document.querySelector('nav').offsetHeight;
    const offset = headerHeight + navHeight + 20;

    navLinks.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            const targetId = this.getAttribute('href').substring(1);
            const targetElement = document.getElementById(targetId);

            if (targetElement) {
                const elementPosition = targetElement.getBoundingClientRect().top + window.pageYOffset;
                const offsetPosition = elementPosition - offset;
                
                window.scrollTo({
                    top: offsetPosition,
                    behavior: 'smooth'
                });
            }
        });
    });
});