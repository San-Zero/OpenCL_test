#!/bin/bash
export SERVICE=vault-server-tls
export NAMESPACE=default
export SECRET_NAME=vault-server-tls
export CA_DIR=vault_ca
export CSR_NAME=vault-csr

mkdir -p ${CA_DIR}
sudo chmod -R 777 ${CA_DIR}
openssl genrsa -out ${CA_DIR}/vault.key 2048

cat <<EOF >${CA_DIR}/csr.conf
[req]
req_extensions = v3_req
distinguished_name = req_distinguished_name
[req_distinguished_name]
[ v3_req ]
basicConstraints = CA:FALSE
keyUsage = nonRepudiation, digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth
subjectAltName = @alt_names
[alt_names]
DNS.1 = *.${SERVICE}
DNS.2 = *.${SERVICE}.${NAMESPACE}
DNS.3 = *.${SERVICE}.${NAMESPACE}.svc
DNS.4 = *.${SERVICE}.${NAMESPACE}.svc.cluster.local
DNS.5 = vault-csi
DNS.6 = vault-csi-provider
DNS.7 = vault.csi-*
DNS.8 = vault
DNS.9 = vault-*
DNS.10 = vault*
IP.1 = 127.0.0.1
IP.2 = ${ip}
EOF

openssl req -new \
            -key ${CA_DIR}/vault.key \
            -subj "/CN=system:node:${SERVICE}.${NAMESPACE}.svc;/O=system:nodes" \
            -out ${CA_DIR}/server.csr \
            -config ${CA_DIR}/csr.conf

cat <<EOF >${CA_DIR}/csr.yaml
apiVersion: certificates.k8s.io/v1
kind: CertificateSigningRequest
metadata:
  name: ${CSR_NAME}
spec:
  signerName: kubernetes.io/kubelet-serving
  groups:
  - system:authenticated
  request: $(base64 ${CA_DIR}/server.csr | tr -d '\n')
  signerName: kubernetes.io/kubelet-serving
  usages:
  - digital signature
  - key encipherment
  - server auth
EOF

kubectl create -f ${CA_DIR}/csr.yaml

kubectl certificate approve ${CSR_NAME}

kubectl get csr ${CSR_NAME}

serverCert=$(kubectl get csr ${CSR_NAME} -o jsonpath='{.status.certificate}')

echo "${serverCert}" | openssl base64 -d -A -out ${CA_DIR}/vault.crt

kubectl config view \
--raw \
--minify \
--flatten \
-o jsonpath='{.clusters[].cluster.certificate-authority-data}' \
| base64 -d > ${CA_DIR}/vault.ca

kubectl create secret generic ${SECRET_NAME} \
    --namespace ${NAMESPACE} \
    --from-file=vault.key=${CA_DIR}/vault.key \
    --from-file=vault.crt=${CA_DIR}/vault.crt \
    --from-file=vault.ca=${CA_DIR}/vault.ca
