import mechanize

def read_password(file_path):
    with open(file_path, 'r') as file:
        password = file.readline().strip()
    return password

target = str(input("Enter the target web >> "))
username = input("Enter the username target >> ")

br = mechanize.Browser()

br.open(target)
br.select_form(nr=0)

br["username"] = username

password = read_password('password.txt')
br["password"] = password

br.submit()

print(br.response().read())