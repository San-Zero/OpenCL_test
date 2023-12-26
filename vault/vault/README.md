
k8s_config/vault/

	auto_unseal/ 自動解封的腳本
	secrets/ 上傳到vault的機敏資料(json)
	setting/ vault內部身分權限管理的設定檔

	csi-ca.yaml 根據此yaml的設定，從vault mount CA 到目標pod(alertmanager、node-exporter、postgres)
	csi-oam.yaml 根據此yaml的設定，從vault mount OAM的設定到目標pod
	csi-postgres.yaml 根據此yaml的設定，從vault mount postgres到目標pod
	vault-sa.yaml vault專用的service account
	vault-sc.yaml vault專用的storage class
	vault-service.yaml vault專用的service


secrets.json說明

	oam-secrets.json
		tls.crt、tls.key: oam、alertmanager、node-exporter、postgres之間加密通訊時，使用的CA
		vault.crt、vault.key: oam與vault加密通訊時，使用的CA
		setting.json: oam網頁使用的設定檔，從vault mount資料到pod內，會直接寫入整個json，包含以下資料
			prometheus_url
			postgres
				host
				port
				name
				user
				password
			vault_url
			test_url
			license_agent_url
			five_gs_version
			ssh_port

	postgres-config-secrets.json
		POSTGRES_DB: 資料庫名稱
		POSTGRES_USER: 資料庫的使用者名稱
		POSTGRES_PASSWORD: 資料庫的使用者密碼
		DATA_SOURCE_URI: 資料庫的URI