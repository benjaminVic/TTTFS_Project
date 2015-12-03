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
echo '  > tfs_partition ...'
rm /usr/local/bin/tfs_partition
rm -rf /opt/tfs_partition/
echo '  > tfs_analyze ...'
rm /usr/local/bin/tfs_analyze
rm -rf /opt/tfs_analyze/
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='