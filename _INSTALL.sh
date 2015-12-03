#!/bin/bash
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' Installation des outils TTTFS'
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
echo ' - Installation des librairies'
echo '  > libll.so ...'
#cp ll/bin/libll.so /usr/lib
echo ' --------------------------------------------------------'
echo ' - Installation des commandes'
echo '  > tfs_create ...'
cp -rf cmds/tfs_create/ /opt/tfs_create/
ln -s /opt/tfs_create/bin/tfs_create /usr/local/bin/tfs_create
echo '  > tfs_partition ...'
cp -rf cmds/tfs_partition/ /opt/tfs_partition/
ln -s /opt/tfs_partition/bin/tfs_partition /usr/local/bin/tfs_partition
echo '  > tfs_analyze ...'
cp -rf cmds/tfs_analyze/ /opt/tfs_analyze/
ln -s /opt/tfs_analyze/bin/tfs_analyze /usr/local/bin/tfs_analyze
echo '=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-='
