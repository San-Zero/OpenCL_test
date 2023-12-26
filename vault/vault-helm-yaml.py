import requests
from zipfile import ZipFile
import os
import yaml


config = """
ui = true

listener "tcp" {
    tls_disable = 0
    address = "[::]:8200"
    cluster_address = "[::]:8201"
    tls_cert_file = "/vault/userconfig/vault-server-tls/vault.crt"
    tls_key_file  = "/vault/userconfig/vault-server-tls/vault.key"
    tls_client_ca_file = "/vault/userconfig/vault-server-tls/vault.ca"
}
storage "file" {
    path = "/vault/data"
}
"""

vault_helm_yaml = {
    "global": {
        "tlsDisable": False,
    },
    "server": {
        "dataStorage": {
            "storageClass": "vault-sc"
        },
        "extraEnvironmentVars": {
            "VAULT_CACERT": "/vault/userconfig/vault-server-tls/vault.ca"
        },
        "volumes": [
            {
                "name": "userconfig-vault-server-tls",
                "secret": {
                        "defaultMode": 420,
                        "secretName": "vault-server-tls"
                }
            }
        ],
        "volumeMounts": [
            {
                "mountPath": "/vault/userconfig/vault-server-tls",
                "name": "userconfig-vault-server-tls",
                "readOnly": True
            }
        ],
        "standalone": {
            "enabled": True,
            "config": config
        }
    },
    "injector": {
        "enabled": False,
    },
    "csi": {
        "enabled": True,
        "volumes": [
            {
                "name": "userconfig-vault-server-tls",
                "secret": {
                        "defaultMode": 420,
                        "secretName": "vault-server-tls"
                }
            }
        ],
        "volumeMounts": [
            {
                "mountPath": "/vault/userconfig/vault-server-tls",
                "name": "userconfig-vault-server-tls",
                "readOnly": True
            }
        ]
    }
}


def vaultHelmYamlFormat(version: str, unzipPath: str):
    vault_helm_git_url = f"https://github.com/hashicorp/vault-helm/archive/refs/tags/v{version}.zip"

    file_name = f"vault-helm-{version}"
    file_path = f"{unzipPath}/{file_name}"

    if not os.path.exists(file_path+".zip"):
        response = requests.get(vault_helm_git_url)
        with open(file_path+".zip", 'wb') as f:
            f.write(response.content)

    if not os.path.exists(file_path):
        with ZipFile(file_path+".zip", 'r') as z:
            z.extractall(path=unzipPath)

    with open(f"{file_path}/values.yaml") as f:
        data = yaml.load(f, Loader=yaml.FullLoader)

    vault_image = f'{data["server"]["image"]["repository"]}:{data["server"]["image"]["tag"]}'
    vault_csi_provider = f'{data["csi"]["image"]["repository"]}:{data["csi"]["image"]["tag"]}'

    # os.system(f"sudo docker pull {vault_image}")
    # os.system(f"sudo docker pull {vault_csi_provider}")

    data["global"]["tlsDisable"] = vault_helm_yaml["global"]["tlsDisable"]
    data["server"]["dataStorage"]["storageClass"] = vault_helm_yaml["server"]["dataStorage"]["storageClass"]
    data["server"]["extraEnvironmentVars"] = vault_helm_yaml["server"]["extraEnvironmentVars"]
    data["server"]["volumes"] = vault_helm_yaml["server"]["volumes"]
    data["server"]["volumeMounts"] = vault_helm_yaml["server"]["volumeMounts"]
    data["server"]["standalone"] = vault_helm_yaml["server"]["standalone"]
    data["injector"]["enabled"] = vault_helm_yaml["injector"]["enabled"]
    data["csi"]["enabled"] = vault_helm_yaml["csi"]["enabled"]
    data["csi"]["volumes"] = vault_helm_yaml["csi"]["volumes"]
    data["csi"]["volumeMounts"] = vault_helm_yaml["csi"]["volumeMounts"]

    with open(f"{file_path}/values.yaml", "w") as f:
        yaml.safe_dump(data, f, sort_keys=False,
                       default_flow_style=False,
                       default_style='|')


if __name__ == "__main__":
    vaultHelmYamlFormat(version="0.27.0", unzipPath="/tmp")

#!/bin/bash

# # 升級vault
# helm upgrade vault /tmp/vault-helm-0.25.0

# VAULT_UNSEAL_KEY=$(jq -r ".unseal_keys_b64[]" k8s_config/vault/cluster-keys.json) # 原先的unseal_key
# VAULT_POD_NAME=vault-0

# kubectl delete pod $VAULT_POD_NAME

# VAULT_STATUS=$(kubectl get pod | grep $VAULT_POD_NAME | awk '{print $3}')
# while [ "$VAULT_STATUS" != "Running" ]; do
#     VAULT_STATUS=$(kubectl get pod | grep $VAULT_POD_NAME | awk '{print $3}')
#     echo "$VAULT_POD_NAME is not Running yet. Waiting..."
#     sleep 2
# done

# # 解封
# kubectl exec $VAULT_POD_NAME -- vault operator unseal $VAULT_UNSEAL_KEY
