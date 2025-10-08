import cv2
import os
import time

# Chemin vers le répertoire où les images sont sauvegardées
IMAGE_DIRECTORY = "./images/"
RESULT_FILE = "varroa_counts.txt"
processed_files = set()

def process_image(image_path):
    """
    Traite une image pour détecter et compter les varroas.
    """
    # Charger l'image
    image = cv2.imread(image_path)
    if image is None:
        print(f"Impossible de charger l'image : {image_path}")
        return

    # Conversion en niveaux de gris
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    # Seuil adaptatif pour la binarisation
    binary = cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
                                   cv2.THRESH_BINARY_INV, 11, 2)
    
    # Détection des contours
    contours, _ = cv2.findContours(binary, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Filtrer les contours et compter les varroas
    varroa_count = 0
    for contour in contours:
        area = cv2.contourArea(contour)
        perimeter = cv2.arcLength(contour, True)
        circularity = 4 * 3.14 * (area / (perimeter ** 2)) if perimeter > 0 else 0

        if 50 < area < 500 and 0.7 < circularity < 1.2:  # Ajuster ces valeurs
            varroa_count += 1
            cv2.drawContours(image, [contour], -1, (0, 255, 0), 2)
    
    # Affichage des images pour vérification
    cv2.imshow("Image Originale", image)
    cv2.imshow("Binarisation", binary)
    cv2.waitKey(500)  # Attend 500 ms avant de fermer les fenêtres
    cv2.destroyAllWindows()
    
    # Sauvegarder l'image traitée
    output_path = f"processed_{os.path.basename(image_path)}"
    cv2.imwrite(output_path, image)
    
    # Enregistrer les résultats
    with open(RESULT_FILE, "a") as f:
        f.write(f"{image_path} : {varroa_count} varroas\n")
    
    print(f"Image traitée : {output_path}")
    print(f"Nombre de varroas détectés : {varroa_count}")

# Analyse automatique des nouvelles images
while True:
    for filename in os.listdir(IMAGE_DIRECTORY):
        if filename.endswith(".jpg") and filename not in processed_files:
            process_image(os.path.join(IMAGE_DIRECTORY, filename))
            processed_files.add(filename)
    time.sleep(5)  # Vérifie toutes les 5 secondes