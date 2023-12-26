#!/bin/bash

# 升級vault的image(latest)
helm upgrade vault k8s_config/helm/vault-helm-0.25.0 \
    --set "server.dev.enabled=false" \
    --set "injector.enabled=false" \
    --set "csi.enabled=true" \
    --set "server.dataStorage.storageClass=vault-sc" \
    --set "server.image.tag=latest"

VAULT_UNSEAL_KEY=$(jq -r ".unseal_keys_b64[]" k8s_config/vault/cluster-keys.json) # 原先的unseal_key
VAULT_POD_NAME=vault-0

kubectl delete pod $VAULT_POD_NAME

VAULT_STATUS=$(kubectl get pod | grep $VAULT_POD_NAME | awk '{print $3}')
while [ "$VAULT_STATUS" != "Running" ]; do
    VAULT_STATUS=$(kubectl get pod | grep $VAULT_POD_NAME | awk '{print $3}')
    echo "$VAULT_POD_NAME is not Running yet. Waiting..."
    sleep 2
done

# 解封
kubectl exec $VAULT_POD_NAME -- vault operator unseal $VAULT_UNSEAL_KEY
