<?php

use Nette\Application\UI\Form;

/*
 * presenter pro registraci uzivatelu
 */
class RegisterPresenter extends BasePresenter
{
    private $userRepository;
    private $authenticator;

    protected function startup()
    {
	parent::startup();
	$this->userRepository = $this->context->userRepository;
	$this->authenticator = $this->context->authenticator;
    }

    public function renderDefault()
    {
    }

    protected function createComponentRegisterForm() 
    {
	$form = new Form;
	$form->addText('fullname', 'Vaše celé jméno:');
	$form->addText('username', 'Uživatelské jmeno:')
	    ->setRequired('Musíte zadat vaše uživatelské jméno.');
	$form->addPassword('password', 'Heslo:')
		->addRule(Form::FILLED, 'Zadejte prosím heslo.')
	    ->addRule(Form::MIN_LENGTH, 'Nové heslo musí mít alespoň %d znaku', 5);
	$form->addPassword('confirmpass', 'Potvrzení hesla:')
	    ->addRule(Form::FILLED, 'Zadejte prosím znovu své heslo pro potvrzení.')
	    ->addRule(Form::EQUAL, 'Zadaná hesla musí být stejná.', $form['password']);
	$form->addSubmit('register', 'Registrovat');
	$form->onSuccess[] = $this->registerFormSubmitted;
	return $form;
    }

    public function registerFormSubmitted(Form $form)
    {
	$values = $form->getValues();
	$row = $this->userRepository->findByName($values->username);
	if ($row) { // uzivatel existuje, vyhodime vyjimku
	    $form->addError('Uživatel se zadaným jménem již existuje, zvolte prosím jiné uživatelské jméno');
	    return;
	}
	$this->userRepository->addUser(
	    $values->username, 
	    $values->password, 
	    empty($values->fullname) ? '' : $values->fullname);
	$this->redirect('Register:success');
    }
}
