#!/bin/bash

SLEEP_TIME=5
MAX_RETRIES=100

for ((i=1; i<=$MAX_RETRIES; i++)); do
    kubectl get nodes > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "Waiting for Kubernetes to be ready..."
        sleep $SLEEP_TIME
        continue
    fi

    VAULT_STATUS=$(kubectl get pod | grep POD_NAME | awk '{print $3}')
    if [ "$VAULT_STATUS" != "Running" ]; then
        echo "POD_NAME is not Running yet. Waiting..."
        sleep $SLEEP_TIME
        continue
    fi

    OAM_STATUS=$(kubectl get pod | grep oam-deployment | awk '{print $3}')
    POSTGRES_STATUS=$(kubectl get pod | grep postgres-deployment | awk '{print $3}')
    if [ "$OAM_STATUS" != "Running" ] || [ "$POSTGRES_STATUS" != "Running" ]; then
        echo "OAM or POSTGRES is not Running yet. Waiting..."
        kubectl exec POD_NAME -- vault operator unseal UNSEAL_KEY
        sleep $SLEEP_TIME
        continue
    fi
done

unset SLEEP_TIME
unset MAX_RETRIES
