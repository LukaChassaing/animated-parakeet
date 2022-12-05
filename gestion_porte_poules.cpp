#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <ctime>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <iomanip>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <unistd.h>
#include <wiringPi.h>

using namespace std;

int pinRelai_1 = 4;
int pinRelai_2 = 5;
int duree_ouverture = 90000000;
int duree_fermeture = 90000000;


void pousse_verrin(){
	std::cout << "Pousse verrin\n";
	digitalWrite(pinRelai_1, 1);
    digitalWrite(pinRelai_2, 0);
}

void tire_verrin(){
	std::cout << "Tire verrin\n";
	digitalWrite(pinRelai_1, 0);
    digitalWrite(pinRelai_2, 1);
}

void coupe_alimention_verrin(){
	std::cout << "Coupe alimentation verrin\n";
	digitalWrite(pinRelai_1, 0);
    digitalWrite(pinRelai_2, 0);
}

void ouverture_porte(){
	std::cout << "\nOuverture porte\n";
	tire_verrin();
	usleep(duree_ouverture);
	coupe_alimention_verrin();
}

void fermeture_porte(){
	std::cout << "\nFermeture porte\n";
	pousse_verrin();
	usleep(duree_fermeture);
	coupe_alimention_verrin();
}

std::vector<std::string> explode(char delim, std::string const & s)
{
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); )
    {
        result.push_back(std::move(token));
    }

    return result;
}

int main(void)
{
    std::cout << "Lancement du programme de gestion de la porte des poules\n";
	if(wiringPiSetup() < 0){
		std::cout << "Impossible d'initialiser wiringPi\n";
		return 1;
	}

	pinMode(pinRelai_1, OUTPUT);
    pinMode(pinRelai_2, OUTPUT);

	usleep(100000);

	digitalWrite(pinRelai_1, 1);
    digitalWrite(pinRelai_2, 1);
	usleep(100000);
	digitalWrite(pinRelai_1, 0);
    digitalWrite(pinRelai_2, 0);
	usleep(100000);
	digitalWrite(pinRelai_1, 1);
    digitalWrite(pinRelai_2, 1);
	usleep(100000);
	digitalWrite(pinRelai_1, 0);
    digitalWrite(pinRelai_2, 0);

	usleep(5000000);


	
	sql::Driver* driver;
	sql::Connection* connexion_bdd;
	sql::Statement* stmt;
	sql::ResultSet* res;

	try
	{
		driver = get_driver_instance();
	}
	catch (sql::SQLException e)
	{
		std::cout << "Impossible de récupérer le driver bdd. Message d'erreur : " << e.what() << "\n";
		return 1;
	}

	try
    {
		connexion_bdd = driver->connect("tcp://ip:port", "login", "password");
		connexion_bdd->setSchema("GESTION_POULES");
	}
	catch (sql::SQLException e)
	{
		std::cout << "Impossible de se connecter à la base de données. Message d'erreur : " << e.what() << "\n";
		return 1;
	}

	stmt = connexion_bdd->createStatement();

	while(1){
		res = stmt->executeQuery("SELECT * FROM PORTES");

		int compteur_debut_jour = 0;
		int compteur_debut_nuit = 0;
		int compteur_actuel = 0;
		int ouvert = 0;
		int ferme = 0;

		std::cout << "\n";

		time_t datetime_actuel = time(0);
		tm *ltm = localtime(&datetime_actuel);
		char datetime_actuel_formate[30];
		sprintf(datetime_actuel_formate, "%04d-%02d-%02d %02d:%02d:%02d", (1900 + ltm->tm_year), (1 + ltm->tm_mon), ltm->tm_mday, 1+ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

		compteur_actuel = ((1+ltm->tm_hour)*60)+ltm->tm_min;

		std::cout << "-----------------------------------------\n";
		std::cout << "Datetime actuel : " << datetime_actuel_formate << "\n";
		std::cout << "\nConfiguration récupérée de la bdd \n";
	
		while (res->next()) {
			auto v = explode(':', res->getString("debut_jour"));
			auto w = explode(' ', v[0]);
			std::cout << "\t debut_jour : " << w[1] << ":" << v[1] << "\n";
			compteur_debut_jour = (stoi(w[1])*60)+stoi(v[1]);

			auto x = explode(':', res->getString("debut_nuit"));
			auto y = explode(' ', x[0]);
			std::cout << "\t debut_nuit : " << y[1] << ":" << x[1] << "\n";
			compteur_debut_nuit = (stoi(y[1])*60)+stoi(x[1]);

			std::cout << "\t ouvert : " << res->getInt("ouvert") << "\n";
			std::cout << "\t ferme : " << res->getInt("ferme") << "\n";

			ouvert = res->getInt("ouvert");
			ferme = res->getInt("ferme");
		}
		delete res;

		std::cout << "\ncompteur_debut_jour : " << compteur_debut_jour << "\n";
		std::cout << "compteur_debut_nuit : " << compteur_debut_nuit << "\n";
		std::cout << "compteur_actuel : " << compteur_actuel << "\n";

		std::string phase_actuelle = "";
		if(compteur_actuel < compteur_debut_jour){
			phase_actuelle = "nuit";
		}
		else if(compteur_actuel > compteur_debut_jour && compteur_actuel < compteur_debut_nuit){
			phase_actuelle = "jour";
		}
		else if(compteur_actuel > compteur_debut_nuit){
			phase_actuelle = "nuit";
		}

		std::cout << "phase actuelle : " << phase_actuelle << "\n"; 

		if(phase_actuelle == "jour" && ouvert == 0){
			ouverture_porte();
			stmt->execute("UPDATE PORTES SET ouvert=1, ferme=0 WHERE id=1");
		}

		if(phase_actuelle == "nuit" && ferme == 0){
			fermeture_porte();
			stmt->execute("UPDATE PORTES SET ferme=1, ouvert=0 WHERE id=1");
		}
		usleep(10000000);
	}
	delete connexion_bdd;
	return 0;
}