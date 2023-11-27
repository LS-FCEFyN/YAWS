document.addEventListener("DOMContentLoaded", function () {
    const projects = [
        {
            title: "YAWS",
            description: "Yet Another Web Server<br>A simple web server written in C<br>It's the server that's powering this site!",
            logo: "/media/YAWS.jpeg"
        }
    ];

    const projectContainer = document.getElementById("project-container");

    projects.forEach((project, index) => {
        const projectCard = document.createElement("div");
        projectCard.classList.add("project-card");
        projectCard.innerHTML = `
            <img src="${project.logo}" alt="Project Logo" class="project-logo">
            <h3>${project.title}</h3>
            <p>${project.description}</p>
        `;
    
        projectCard.style.animation = `fadeIn 1s ${index * 0.2}s forwards`;
        projectContainer.appendChild(projectCard);
    });

});
