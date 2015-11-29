#!/bin/bash
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' Installation des outils TTTFS'
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' - Installation des librairies'
echo ' --------------------------------------------------------'
echo ' - Installation des commandes'
echo '  > tfs_create ...'
cp -rf Commandes/tfs_create/ /opt/tfs_create/
ln -s /opt/tfs_create/bin/tfs_create /usr/local/bin/tfs_create
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
