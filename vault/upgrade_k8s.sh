#!/bin/bash
UPGRADE_VERSION=v1.27
USER=let02

CONTROLPLANE_NAME=$(sudo -u $USER kubectl get nodes | grep control-plane | awk '{print $1}')

curl -fsSL https://pkgs.k8s.io/core:/stable:/$UPGRADE_VERSION/deb/Release.key | sudo gpg --yes --dearmor -o /etc/apt/keyrings/kubernetes-apt-keyring.gpg

echo 'deb [signed-by=/etc/apt/keyrings/kubernetes-apt-keyring.gpg] https://pkgs.k8s.io/core:/stable:/'"$UPGRADE_VERSION"'/deb/ /' | sudo tee /etc/apt/sources.list.d/kubernetes.list

sudo apt-mark unhold kubelet kubeadm kubectl
sudo apt-get update -y
sudo apt-get install -y kubeadm
sudo apt-mark hold kubeadm

# get what excatly version of kubeadm is installed
OLD_VERSION=$(sudo -u $USER kubectl get nodes -o=jsonpath='{.items[0].status.nodeInfo.kubeletVersion}')
INSTALLED_VERSION=$(apt-cache policy kubeadm | awk '/Installed:/{print $2}' | cut -d- -f1)

if [ "$OLD_VERSION" != "v$INSTALLED_VERSION" ]; then
    sudo -u $USER kubectl drain $CONTROLPLANE_NAME --ignore-daemonsets --delete-emptydir-data --force
    sudo kubeadm upgrade plan $INSTALLED_VERSION
    sudo kubeadm upgrade apply $INSTALLED_VERSION -y
    sudo -u $USER kubectl uncordon $CONTROLPLANE_NAME
    # upgrade kubelet kubectl
    sudo apt-get install -y kubelet kubectl
    sudo apt-mark hold kubelet kubectl
    sudo systemctl daemon-reload
    sudo systemctl restart kubelet

    echo "1" > /tmp/update_k8s_version
else
    echo "0" > /tmp/update_k8s_version
fi
