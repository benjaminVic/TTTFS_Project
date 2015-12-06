#!/bin/bash
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' Installation des outils TTTFS'
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' - Copie des fichiers dans /opt/TTTFS/ ...'
cp -rf . /opt/TTTFS
echo ' - Création du répertoire /tfs_disks/ ...'
mkdir /tfs_disks
echo ' --------------------------------------------------------'
echo ' - Installation des librairies'
echo '  > libll.so ...'
#cp ll/bin/libll.so /usr/lib
echo ' --------------------------------------------------------'
echo ' - Installation des commandes'
echo '  > tfs_create ...'
ln -s /opt/TTTFS/bin/tfs_create /usr/local/bin/tfs_create
echo '  > tfs_partition ...'
ln -s /opt/TTTFS/bin/tfs_partition /usr/local/bin/tfs_partition
echo '  > tfs_analyze ...'
ln -s /opt/TTTFS/bin/tfs_analyze /usr/local/bin/tfs_analyze
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
