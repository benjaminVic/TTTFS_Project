#!/bin/bash
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' Suppression des outils TTTFS'
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' - Suppression de /opt/TTTFS/ ...'
rm -rf /opt/TTTFS/
echo ' - Suppression du répertoire /tfs_disks/ ...'
rm -rf /tfs_disks
echo ' --------------------------------------------------------'
echo ' - Suppression des librairies'
echo '  > libll.so ...'
#rm /usr/lib/libll.so
echo ' --------------------------------------------------------'
echo ' - Suppression des commandes'
echo '  > tfs_create ...'
rm /usr/local/bin/tfs_create
echo '  > tfs_partition ...'
rm /usr/local/bin/tfs_partition
echo '  > tfs_analyze ...'
rm /usr/local/bin/tfs_analyze
echo '  > tfs_format ...'
rm /usr/local/bin/tfs_format
echo '  > tfs_mkdir ...'
rm /usr/local/bin/tfs_mkdir
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
