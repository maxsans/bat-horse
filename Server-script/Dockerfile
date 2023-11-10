# Utilisez une image de base avec Python
FROM python:3.8

# Répertoire de travail dans le conteneur
WORKDIR /app

# Copiez le fichier de dépendances dans le conteneur
COPY requirements.txt .

# Installez les dépendances
RUN pip install --no-cache-dir -r requirements.txt

# Copiez le reste du code de l'application dans le conteneur
COPY . .

# Exposez le port que l'application Flask va écouter
EXPOSE 3005

# Commande pour démarrer l'application Flask
CMD ["python", "server.py"]
