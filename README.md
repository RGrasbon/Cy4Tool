# Cy4Tool

Ein leistungsstarkes PC-Werkzeug für die Administration, Konfiguration und Instandhaltung von **Cy4NET-Komponenten** des freien Peer-To-Peer Kommunikationssystems Cy4NET: https://cy4net.org

## Über die Anwendung

**Cy4Tool** wurde entwickelt, um Anwendern die Handhabung von Cy4NET-Buskomponenten in einem Cy4NET-Netzwerk zu erleichtern. Die Anwendung bündelt wichtige Diagnose-, Wartungs- und Verwaltungsfunktionen in einer übersichtlichen Desktop-Oberfläche. Um mit den Teilnehmern des Cy4NET-Netzwerks in Kontakt zu treten, verbindet sich die Anwendung als Busclient über ein entsprechendes Gateway-Gerät mit dem Bus. Je nach vorhandener Hardware kann die Verbindung mittels RS232 oder TCP hergestellt werden. 
Das Cy4NET-Protokoll sieht für die Eigenschaften und Funktionen einer Anwendung die Ausführung von Diensten vor. Diese Dienste sind im Grunde eine Abstraktion von Speicherbereichen, die beschrieben oder gelesen werden können. Welche Informationen und Datenstrukturen dabei übertragen werden, ist jeweils mit dem Dienst definiert. Mithilfe des Cy4Tools können diese Dienste zur Kontaktierung der Busknoten genutzt werden.
Die Funktionalität einer Anwendung wird dabei durch eine sogenannte Dienstekarte (Servicemap) abgebildet. Neben den Diensten der Anwendung ist darin auch eine Oberfläche mit Steuerelementen und Widgets definiert, die den Zugriff auf die Komponenten erleichtern. Das Cy4Tool ist somit das Frontend der Cy4NET-Geräte und als basisnahe Serviceschnittstelle zu verstehen.

Auf der Cy4NET-Projekteseite sind mehrere Gateways zu finden. Darunter auch Geräte, die die Funktion eines Gateways als Zusatzleistung bereitstellen.
https://cy4net.org/Axxxxx/A00111_GatewayMonitor/A00111Home.html
https://cy4net.org/Axxxxx/A00121_Powerstation/A00121Home.html
https://cy4net.org/Axxxxx/A00317_Evalboard644PKit/A00317Home.html

### Hauptfunktionen
* **Komponenten-Verwaltung:** Schnelles Auslesen und Anpassen von Konfigurationen für angeschlossene Cy4NET-Geräte.
* **Instandhaltung & Diagnose:** Integrierte Werkzeuge zur Fehleranalyse, Statusüberwachung und Protokollauswertung.
* **Firmware-Management:** Einfaches Durchführen von Updates und Backups direkt vom PC aus.
* **[Weitere Funktion]:** [Kurze Beschreibung hinzufügen, z. B. Automatische Netzwerk-Erkennung].

## Voraussetzungen

Um Cy4Tool nutzen zu können, muss dein System folgende Anforderungen erfüllen:
* **Betriebssystem:** [z. B. Windows 10/11, Linux (Ubuntu 22.04+) oder FreeBSD]
* **Systemanforderungen:** [z. B. Mindestens 4 GB RAM, 100 MB freier Speicherplatz]
* **Zusätzliche Software:** [z. B. .NET Runtime 8.0 / Python 3.10+ / USB-Treiber – falls nicht benötigt, diesen Punkt einfach löschen]

## Installation

1. Lade die neueste Version aus dem Bereich [Releases](../../releases) herunter oder klone das Repository:
   ```bash
   git clone https://github.com[Dein-Benutzername]/Cy4Tool.git
   ```
2. Wechsle in das Projektverzeichnis:
   ```bash
   cd Cy4Tool
   ```
3. [Installationsschritt anpassen, z. B.: Starte die `Cy4Tool.exe` direkt ohne Installation / Führe `make install` aus].

## Nutzung

1. Verbinde die gewünschte **Cy4NET-Komponente** über [Netzwerk / USB / serielle Schnittstelle] mit deinem PC.
2. Starte **Cy4Tool**.
3. Wähle im Hauptmenü das gewünschte Gerät aus, um den aktuellen Status einzusehen oder Wartungsaufgaben zu starten.

*(Tipp: Füge hier später einen Screenshot deiner Anwendung ein, z. B. mit `![Cy4Tool Oberfläche](pfad/zum/screenshot.png)`)*

## Mitwirken (Contributing)

Beiträge und Verbesserungsvorschläge sind willkommen! Wenn du einen Fehler findest oder eine Idee für eine neue Funktion hast, erstelle gerne ein **Issue** oder reiche einen **Pull Request** ein.

## Lizenz

Dieses Projekt steht unter der **FreeBSD-Lizenz** (2-Clause BSD License). Weitere Details findest du in der Datei [LICENSE](LICENSE) im Hauptverzeichnis dieses Repositories.
