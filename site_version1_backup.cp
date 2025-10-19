---
- hosts: all
  become: true
  pre_tasks:

    - name: update system (AlmaLinux)
      tags: always
      dnf:
        name: "*"
        update_only: yes
        update_cache: yes
        state: latest
      when: ansible_distribution == "AlmaLinux"

    - name: update system (Debian)
      tags: always
      apt:
        upgrade: dist
        update_cache: yes
      when: ansible_distribution in ["Ubuntu", "Debian"]

- hosts: all
  become: true
  tasks:

    - name: create simone user
      tags: always
      user:
        name: simone
        groups: root

    - name: add ssh key for simone
      tags: always
      authorized_key:
        user: simone
        key: "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIJkruS3bM+KLWEKDiBg4i6IMJHLglEO0b7+EI/iCSnaX ansible"

    - name: add sudoers file for simone
      tags: always
      copy:
        src: sudoer_simone
        dest: /etc/sudoers.d/simone
        owner: root
        group: root
        mode: 0440

- hosts: workstations
  become: true
  tasks:

    - name: install unzip
      package:
        name: unzip
        state: present

    - name: install terraform
      unarchive:
        src: https://releases.hashicorp.com/terraform/1.13.4/terraform_1.13.4_linux_amd64.zip
        dest: /usr/local/bin
        remote_src: yes
        owner: root
        group: root
        mode: '0755'


- hosts: web_servers
  become: true
  tasks:

    - name: install apache and php for Debian servers
      tags: apache,apache2,Debian
      apt:
        name:
          - apache2
          - libapache2-mod-php
        state: latest
      when: ansible_distribution in ["Ubuntu", "Debian"]

    - name: install apache and php for AlmaLinux servers
      tags: apache,Almalinux,httpd
      dnf:
        name:
          - httpd
          - php
        state: latest
      when: ansible_distribution == "AlmaLinux"

    - name: start httpd (almaLinux)
      tags: apache,Almalinux,httpd
      service:
        name: httpd
        state: started
        enabled: yes
      when: ansible_distribution == "AlmaLinux"

    - name: change e-mail adress for admin
      tags: apache,Almalinux,httpd
      lineinfile:
        path: /etc/httpd/conf/httpd.conf
        regexp: '^ServerAdmin'
        line: ServerAdmin negaluvunga@gmail.com
        backup: yes
      when: ansible_distribution == "AlmaLinux"
      register: httpd

    - name: restart httpd (almaLinux)
      tags: apache,Almalinux,httpd
      service:
        name: httpd
        state: restarted
      when: httpd.changed

    - name: copy defaut html file for site
      tags: apache,Almalinux,httpd
      copy:
        src: default_site.html
        dest: /var/www/html/index.html
        owner: root
        group: root
        mode: 0644

    - name: ensure Apache is running and enabled (Debian)
      service:
        name: apache2
        state: started
        enabled: yes
      when: ansible_distribution in ["Ubuntu", "Debian"]

    - name: ensure Apache is running and enabled (AlmaLinux)
      service:
        name: httpd
        state: started
        enabled: yes
      when: ansible_distribution == "AlmaLinux"

- hosts: db_servers
  become: true
  tasks:

    - name: install mariadb package (Almalinux)
      tags: Almalinux,mariadb,db
      dnf:
        name: mariadb
        state: latest
        update_cache: yes
      when: ansible_distribution == "AlmaLinux"

    - name: install mariadb package (Debian)
      tags: Debian,mariadb,db
      apt:
        name: mariadb-server
        state: latest
        update_cache: yes
      when: ansible_distribution in ["Ubuntu", "Debian"]

- hosts: file_servers
  become: true
  tasks:

    - name: install samba package (Almalinux)
      tags: samba
      package:
        name: samba
        state: latest
#      update_cache: yes
