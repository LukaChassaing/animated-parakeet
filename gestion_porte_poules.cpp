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

const int PIN_RELAI_1 = 4;
const int PIN_RELAI_2 = 5;
const int DUREE_OUVERTURE = 90000000;
const int DUREE_FERMETURE = 90000000;

// Fonction pour pousser le verrin
void pousserVerrin() {
    cout << "Pousse verrin\n";
    digitalWrite(PIN_RELAI_1, 1);
    digitalWrite(PIN_RELAI_2, 0);
}

// Fonction pour tirer le verrin
void tirerVerrin() {
    cout << "Tire verrin\n";
    digitalWrite(PIN_RELAI_1, 0);
    digitalWrite(PIN_RELAI_2, 1);
}

// Fonction pour couper l'alimentation du verrin
void couperAlimentationVerrin() {
    cout << "Coupe alimentation verrin\n";
    digitalWrite(PIN_RELAI_1, 0);
    digitalWrite(PIN_RELAI_2, 0);
}

// Fonction pour ouvrir la porte
void ouvrirPorte() {
    cout << "\nOuverture porte\n";
    tirerVerrin();
    usleep(DUREE_OUVERTURE);
    couperAlimentationVerrin();
}

// Fonction pour fermer la porte
void fermerPorte() {
    cout << "\nFermeture porte\n";
    pousserVerrin();
    usleep(DUREE_FERMETURE);
    couperAlimentationVerrin();
}

// Fonction pour diviser une chaîne de caractères en fonction d'un délimiteur
vector<string> explode(char delim, const string& s) {
    vector<string> result;
    istringstream iss(s);

    for (string token; getline(iss, token, delim); ) {
        result.push_back(move(token));
    }

    return result;
}

int main() {
    cout << "Lancement du programme de gestion de la porte des poules\n";
    
    // Initialisation de wiringPi
    if (wiringPiSetup() < 0) {
        cout << "Impossible d'initialiser wiringPi\n";
        return 1;
    }

    // Configuration des broches en sortie
    pinMode(PIN_RELAI_1, OUTPUT);
    pinMode(PIN_RELAI_2, OUTPUT);

    // Séquence de test des relais
    usleep(100000);
    digitalWrite(PIN_RELAI_1, 1);
    digitalWrite(PIN_RELAI_2, 1);
    usleep(100000);
    digitalWrite(PIN_RELAI_1, 0);
    digitalWrite(PIN_RELAI_2, 0);
    usleep(100000);
    digitalWrite(PIN_RELAI_1, 1);
    digitalWrite(PIN_RELAI_2, 1);
    usleep(100000);
    digitalWrite(PIN_RELAI_1, 0);
    digitalWrite(PIN_RELAI_2, 0);

    usleep(5000000);

    sql::Driver* driver;
    sql::Connection* connexionBdd;
    sql::Statement* stmt;
    sql::ResultSet* res;

    // Récupération du driver de la base de données
    try {
        driver = get_driver_instance();
    }
    catch (sql::SQLException e) {
        cout << "Impossible de récupérer le driver bdd. Message d'erreur : " << e.what() << "\n";
        return 1;
    }

    // Connexion à la base de données
    try {
        connexionBdd = driver->connect("tcp://ip:port", "login", "password");
        connexionBdd->setSchema("GESTION_POULES");
    }
    catch (sql::SQLException e) {
        cout << "Impossible de se connecter à la base de données. Message d'erreur : " << e.what() << "\n";
        return 1;
    }

    stmt = connexionBdd->createStatement();

    while (true) {
        res = stmt->executeQuery("SELECT * FROM PORTES");

        int compteurDebutJour = 0;
        int compteurDebutNuit = 0;
        int compteurActuel = 0;
        int ouvert = 0;
        int ferme = 0;

        cout << "\n";

        time_t datetimeActuel = time(0);
        tm* ltm = localtime(&datetimeActuel);
        char datetimeActuelFormate[30];
        sprintf(datetimeActuelFormate, "%04d-%02d-%02d %02d:%02d:%02d", (1900 + ltm->tm_year), (1 + ltm->tm_mon), ltm->tm_mday, 1 + ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

        compteurActuel = ((1 + ltm->tm_hour) * 60) + ltm->tm_min;

        cout << "-----------------------------------------\n";
        cout << "Datetime actuel : " << datetimeActuelFormate << "\n";
        cout << "\nConfiguration récupérée de la bdd \n";

        // Récupération des données de la base de données
        while (res->next()) {
            auto v = explode(':', res->getString("debut_jour"));
            auto w = explode(' ', v[0]);
            cout << "\t debut_jour : " << w[1] << ":" << v[1] << "\n";
            compteurDebutJour = (stoi(w[1]) * 60) + stoi(v[1]);

            auto x = explode(':', res->getString("debut_nuit"));
            auto y = explode(' ', x[0]);
            cout << "\t debut_nuit : " << y[1] << ":" << x[1] << "\n";
            compteurDebutNuit = (stoi(y[1]) * 60) + stoi(x[1]);

            cout << "\t ouvert : " << res->getInt("ouvert") << "\n";
            cout << "\t ferme : " << res->getInt("ferme") << "\n";

            ouvert = res->getInt("ouvert");
            ferme = res->getInt("ferme");
        }
        delete res;

        cout << "\ncompteurDebutJour : " << compteurDebutJour << "\n";
        cout << "compteurDebutNuit : " << compteurDebutNuit << "\n";
        cout << "compteurActuel : " << compteurActuel << "\n";

        string phaseActuelle = "";
        // Détermination de la phase actuelle (jour ou nuit)
        if (compteurActuel < compteurDebutJour) {
            phaseActuelle = "nuit";
        }
        else if (compteurActuel > compteurDebutJour && compteurActuel < compteurDebutNuit) {
            phaseActuelle = "jour";
        }
        else if (compteurActuel > compteurDebutNuit) {
            phaseActuelle = "nuit";
        }

        cout << "phase actuelle : " << phaseActuelle << "\n";

        // Ouverture de la porte si c'est le jour et qu'elle n'est pas déjà ouverte
        if (phaseActuelle == "jour" && ouvert == 0) {
            ouvrirPorte();
            stmt->execute("UPDATE PORTES SET ouvert=1, ferme=0 WHERE id=1");
        }

        // Fermeture de la porte si c'est la nuit et qu'elle n'est pas déjà fermée
        if (phaseActuelle == "nuit" && ferme == 0) {
            fermerPorte();
            stmt->execute("UPDATE PORTES SET ferme=1, ouvert=0 WHERE id=1");
        }

        usleep(10000000);
    }

    delete connexionBdd;
    return 0;
}
