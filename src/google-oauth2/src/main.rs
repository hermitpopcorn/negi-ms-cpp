use std::{env, process::exit};
use yup_oauth2::ServiceAccountAuthenticator;

#[tokio::main(flavor = "current_thread")]
async fn main() {
	let args: Vec<String> = env::args().collect();
	if args.len() < 3 {
		eprintln!("requires two arguments: path to credentials file and scopes");
		exit(1);
	}

	let credentials_path = &args[1];
	let credentials = match yup_oauth2::read_service_account_key(credentials_path).await {
		Ok(_credentials) => _credentials,
		Err(error) => {
			eprintln!("could not parse service account key: {}", error);
			exit(2);
		}
	};
	let scopes_vector = args[2].split(",").collect::<Vec<&str>>();

	let authenticator = match ServiceAccountAuthenticator::builder(credentials)
		.build()
		.await
	{
		Ok(_authenticator) => _authenticator,
		Err(error) => {
			eprintln!("could not build authenticator: {}", error);
			exit(3);
		}
	};
	let get_token = match authenticator.token(&scopes_vector).await {
		Ok(_get_token) => _get_token,
		Err(error) => {
			eprintln!("failed to get token: {}", error);
			exit(4);
		}
	};

	match get_token.token() {
		Some(token) => println!("{}", token),
		None => {
			eprintln!("token is empty");
			exit(5);
		}
	}

	exit(0);
}
