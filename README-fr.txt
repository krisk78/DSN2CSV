DSNTree est un programme de conversion des fichiers DSN (un standard utilisé en France pour les déclarations sociales des entreprises à destination de l'administration publique).
Il ajoute des nouveaux champs au fichier original. Les blocs parents sont ajoutés devant chaque bloc ainsi que la valeur de leur rubrique clé (la rubrique du bloc censée contenir une valeur unique).

Il ajoute aussi un numéro de séquence en regard de chaque entité contenue dans un même bloc (par exemple le bloc S21.G00.30 'Individu' contient plusieurs individus). Le numéro de séquence est incrémenté dès que le numéro de la rubrique traitée est inférieure à la rubrique précédente du même bloc.

La structure du fichier DSN est définie dans le fichier DSNTree.xml fourni. Son contenu devrait être révisé à chaque nouvelle version de la DSN.
Ce fichier contient la version qu'il représente, la hiérarchie des blocs et pour chaque bloc son nom et sa rubrique clé lorsqu'il ne s'agit pas de la rubrique '001'.
Vous pouvez utiliser plusieurs fichiers DSNTree.xml pour chaque version de la DSN ou chaque utilisation particulière, en fonction de vos besoins (par exemple utiliser l'identifiant entreprise '019' au lieu du NIR '001' pour le bloc 'Individu').

Utilisez le commutateur /? pour afficher l'aide de la commande.

Aucune interface graphique n'est fournie dans cette version. Cependant vous pouvez utiliser des fichiers .bat pour lancer simplement la commande depuis Windows en appliquant des options spécifiques.
Par exemple, créez le fichier DSN2CSV.bat dans le même dossier que le fichier exécutable, avec ce contenu:
  @Echo OFF
  Set _this=%0
  Set _exe=%_this:DSN2CSV.bat=DSNTree.exe%
  %_exe% %* /t
  Pause
En faisant un glisser-déposer de fichiers sur le fichier DSN2CSV.bat, cela aura pour effet de lancer la commande qui va convertir chaque fichier en utilisant l'option de transposition des rubriques /t. Les fichiers sont créés dans le même dossier que les fichiers d'origine.
Vous pouvez créer plusieurs fichiers .bat, faisant appel à des options différentes, que vous pourrez alors utiliser alternativement en fonction de vos besoins.


Historique des versions
  1.02 Première version de production.
  
