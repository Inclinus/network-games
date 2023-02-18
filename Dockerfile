FROM debian:11

WORKDIR /network_games
COPY ./target/server.out /network_games
COPY ./network_games_bdd.sql /network_games
# --------------- Installation gcc & lib mysql ---------------
RUN apt-get update && apt-get upgrade -y && apt-get install -y gcc default-libmysqlclient-dev
# --------------- Installation mariadb ---------------
RUN apt install mariadb-server mariadb-client -y

EXPOSE 4444
ENTRYPOINT service mariadb start && mariadb < network_games_bdd.sql && mariadb -e "CREATE USER 'admin'@'localhost' IDENTIFIED BY 'eGkE656Sa36yFp';" && mariadb -e "GRANT ALL PRIVILEGES ON *.* TO 'admin'@'localhost';" && ./server.out
