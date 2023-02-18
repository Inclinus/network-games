FROM debian:11

WORKDIR /network_games
COPY ./target/server.out /network_games
COPY ./network_games_bdd.sql /network_games
RUN apt-get update && apt-get upgrade -y && apt-get install -y gcc default-libmysqlclient-dev
RUN apt install mariadb-server mariadb-client -y
#RUN service mariadb start
#RUN mariadb -e "CREATE USER 'admin'@'localhost' IDENTIFIED BY 'eGkE656Sa36yFp';"
#RUN GRANT ALL PRIVILEGES ON *.* TO 'admin'@'localhost';
#RUN FLUSH PRIVILEGES;
#RUN CREATE DATABASE network_games_bdd;
#RUN EXIT;
#RUN mysql network_games_bdd < network_games_bdd.sql

#CMD ["./target/server.out"]
EXPOSE 4444
ENTRYPOINT service mariadb start && mariadb < network_games_bdd.sql && mariadb -e "CREATE USER 'admin'@'localhost' IDENTIFIED BY 'eGkE656Sa36yFp';" && mariadb -e "GRANT ALL PRIVILEGES ON *.* TO 'admin'@'localhost';" && ./server.out
