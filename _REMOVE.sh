#!/bin/bash
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' Suppression des outils TTTFS'
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' - Suppression des librairies'
echo ' --------------------------------------------------------'
echo ' - Suppression des commandes'
echo '  > tfs_create ...'
rm /usr/local/bin/tfs_create
rm -rf /opt/tfs_create/
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='