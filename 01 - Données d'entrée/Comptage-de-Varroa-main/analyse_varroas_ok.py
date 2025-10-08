import cv2
import numpy as np
import os
import sqlite3
from datetime import datetime
import sys

# Paramètres optimisés
MIN_AREA = 370       # Réduit pour capturer les petits varroas
MAX_AREA = 615     # Ajusté selon les observations
MIN_CIRCULARITY = 0.6  # Plus permissif
MAX_CIRCULARITY = 1.3
MIN_THRESHOLD = 30  # Seuil minimum pour la binarisation
MAX_THRESHOLD = 255

def detect_varroas(image_path):
    """Détecte les varroas avec un pipeline amélioré"""
    # Chargement image avec vérification
    img = cv2.imread(image_path)
    if img is None:
        print(f"Erreur: Impossible de lire {image_path}")
        return

    # Pré-traitement amélioré
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    
    # Égalisation d'histogramme pour améliorer le contraste
    gray = cv2.equalizeHist(gray)
    
    # Réduction du bruit avec un flou adaptatif
    blurred = cv2.bilateralFilter(gray, 9, 75, 75)
    
    # Seuillage adaptatif amélioré
    binary = cv2.adaptiveThreshold(blurred, 255, 
                                 cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                 cv2.THRESH_BINARY_INV, 25, 10)

    # Opérations morphologiques pour nettoyer l'image
    kernel = np.ones((3,3), np.uint8)
    binary = cv2.morphologyEx(binary, cv2.MORPH_OPEN, kernel, iterations=1)
    binary = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, kernel, iterations=2)

    # Détection des contours avec paramètres optimisés
    contours, _ = cv2.findContours(binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    # Filtrage intelligent des contours
    varroas = []
    for cnt in contours:
        if is_varroa(cnt):
            # Calcul du rectangle englobant
            x,y,w,h = cv2.boundingRect(cnt)
            aspect_ratio = float(w)/h
            
            # Filtre supplémentaire sur le rapport d'aspect
            if 0.7 < aspect_ratio < 1.4:  # Les varroas sont plutôt ronds
                varroas.append(cnt)

    # Post-traitement et visualisation
    output_img = img.copy()
    for varroa in varroas:
        # Dessin plus précis
        cv2.drawContours(output_img, [varroa], -1, (0, 255, 0), 1)
        
        # Ajout d'un rectangle englobant
        x,y,w,h = cv2.boundingRect(varroa)
        cv2.rectangle(output_img, (x,y), (x+w,y+h), (255,0,0), 1)
    
    # Sauvegarde des résultats
    os.makedirs("./processed", exist_ok=True)
    output_path = f"./processed/annotated_{os.path.basename(image_path)}"
    cv2.imwrite(output_path, output_img)
    
    # Enregistrement dans la BDD
    save_to_db(len(varroas), output_path)
    
    # Affichage des statistiques
    print(f"Varroas détectés: {len(varroas)}")
    cv2.imshow('Detection', output_img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

def is_varroa(contour):
    """Filtre amélioré pour identifier les varroas"""
    area = cv2.contourArea(contour)
    if area < MIN_AREA or area > MAX_AREA:
        return False
        
    perimeter = cv2.arcLength(contour, True)
    if perimeter == 0:
        return False
        
    circularity = 4 * np.pi * area / (perimeter ** 2)
    if not (MIN_CIRCULARITY < circularity < MAX_CIRCULARITY):
        return False
        
    # Nouveau critère : solidité (convexité)
    hull = cv2.convexHull(contour)
    hull_area = cv2.contourArea(hull)
    if hull_area == 0:
        return False
        
    solidity = float(area)/hull_area
    if solidity < 0.85:  # Les varroas ont des contours assez convexes
        return False
        
    return True

def save_to_db(count, image_path):
    """Sauvegarde les résultats dans la base de données"""
    conn = sqlite3.connect('varroa.db')
    cursor = conn.cursor()
    
    # Création de la table si elle n'existe pas
    cursor.execute('''CREATE TABLE IF NOT EXISTS counts
                     (id INTEGER PRIMARY KEY AUTOINCREMENT,
                      date TEXT NOT NULL,
                      varroa_count INTEGER NOT NULL,
                      image_path TEXT NOT NULL)''')
    
    cursor.execute(
        "INSERT INTO counts (date, varroa_count, image_path) VALUES (?, ?, ?)",
        (datetime.now().isoformat(), count, image_path)
    )
    conn.commit()
    conn.close()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        detect_varroas(sys.argv[1])
    else:
        print("Usage: python analyse_varroas.py <chemin_image>")
        print("Exemple: python analyse_varroas.py images/ruche1.jpg")