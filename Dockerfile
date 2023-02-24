FROM debian:11

WORKDIR /network_games
COPY ./target/server.out /network_games
COPY ./network_games_bdd.sql /network_games
COPY ./config_mysql.sql /network_games
# --------------- Installation gcc & lib mysql ---------------
RUN apt-get update && apt-get upgrade -y && apt-get install -y gcc default-libmysqlclient-dev
# --------------- Installation mariadb ---------------
RUN apt install mariadb-server mariadb-client -y
RUN sed -i 's/bind-address/bind-address = 0.0.0.0 #/i' /etc/mysql/mariadb.conf.d/50-server.cnf

RUN /etc/init.d/mariadb start && mysql < network_games_bdd.sql
RUN /etc/init.d/mariadb start && mysql -u root --password="" -e "source ./config_mysql.sql"

EXPOSE 4444
EXPOSE 3306
ENTRYPOINT service mariadb start && ./server.out
