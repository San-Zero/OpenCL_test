#!/bin/bash
# https://www.cnblogs.com/meadow/p/15901831.html
# https://developer.hashicorp.com/vault/tutorials/kubernetes/agent-kubernetes

export SERVICE_ACCOUNT_NAME=vault-auth



# binding service account with ClusterRole
cat <<EOF | kubectl apply -f -
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: role-tokenreview-binding
  namespace: default
roleRef:
  apiGroup: rbac.authorization.k8s.io
  kind: ClusterRole
  name: system:auth-delegator
subjects:
  - kind: ServiceAccount
    name: $SERVICE_ACCOUNT_NAME
    namespace: default
EOF

# create new service account
cat <<EOF | kubectl apply -f -
apiVersion: v1
kind: ServiceAccount
metadata:
  name: $SERVICE_ACCOUNT_NAME
  namespace: default
EOF

# create new secrets to store service account token
cat <<EOF | kubectl apply -f -
apiVersion: v1
kind: Secret
metadata:
  name: $SERVICE_ACCOUNT_NAME-secret
  annotations:
    kubernetes.io/service-account.name: $SERVICE_ACCOUNT_NAME
type: kubernetes.io/service-account-token
EOF


VAULT_SA_NAME=$(kubectl get sa $SERVICE_ACCOUNT_NAME -o json | jq -r '.metadata.name')
SA_JWT_TOKEN=$(kubectl get secret "$VAULT_SA_NAME"-secret -o json | jq -r '.data.token' | base64 --decode)

export VAULT_SA_NAME
export SA_JWT_TOKEN
