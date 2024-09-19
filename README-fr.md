# DSN2CSV

## Utilisation

DSN2CSV est un programme de conversion des fichiers DSN (un standard utilisé en France pour les déclarations sociales des entreprises à destination de l'administration publique).<br>
Il compile le fichier original pour mettre en regard de chaque catégorie de bloc l'ensemble de ses blocs parents et leur clé (en général la catégorie du bloc dont la valeur est unique).

Les blocs et leur hiérarchie, les catégories et leurs libellés sont extraits du document officiel [dsn-datatypes-CTXXXX.xlsx](https://www.net-entreprises.fr/declaration/norme-et-documentation-dsn/) mis à disposition par l'administration.<br>
Le fichier dsn-datatypes.cfg définit la configuration qui permet d'exploiter ce fichier, en lui ajoutant notamment la notion de parent manquante dans la feuille 'Header' ainsi
que la définition de catégories clé spécifiques.

La valeur "seq" pour la clé est utilisée lorsqu'il n'existe pas, à priori, de catégorie dont la valeur est unique au sein de tous les enregistrements d'un même bloc.<br>
Dans ce cas un n° de séquence, plutôt que la valeur d'une catégorie, est utilisé comme valeur clé.<br>
Le n° de séquence est incrémenté lorsque la catégorie lue est inférieure ou égale à la précédente du même bloc.

Utilisez le commutateur /? (Windows) ou -h (Unix-like) pour afficher l'aide de la commande.

Aucune interface graphique n'est fournie dans cette version.<br>
Cependant, vous pouvez utiliser des fichiers .bat pour lancer simplement la commande depuis Windows en appliquant des options spécifiques.<br>
Par exemple, créez le fichier DSN2CSV.bat dans le même dossier que le fichier exécutable, avec ce contenu:
```bat
@Echo OFF
Set _this=%0
Set _exe=%_this:DSN2CSV.bat=dsn2csv.exe%
%_exe% %* /r:dsn-datatypes-CT2024.xlsx /f /l /t /v
Pause
```
Les fichiers qui seront déposés sur le fichier de commande DSN2CSV.bat seront automatiquement convertis, ici avec ajout des libellés et en mode transposition.<br>
Les fichiers sont créés dans le même dossier que les fichiers d'origine.<br>
Vous pouvez créer plusieurs fichiers .bat, faisant appel à des options différentes, que vous pourrez alors utiliser alternativement en fonction de vos besoins.

## Maintenance

Il est important de mettre à jour le fichier dsn-datatypes-CTXXXX.xlsx à chaque nouvelle version pour garantir l'exploitation correcte des fichiers DSN de cette nouvelle version.<br>
Des adaptations peuvent être requises dans le fichier de configuration associé dsn-datatypes.cfg.<br>

## Historique des versions

| Version | Description |
|---------|-------------|
| 1.1.0 | 1ère version de production, reprenant globalement le projet DSNTree en lui ajoutant la capacité de construction multi-plateformes. |
| 1.2.0 | Ajout des libellés en se sourçant sur un fichier officiel mis à disposition par l'administration.<br>Clarification du code de conversion. |
