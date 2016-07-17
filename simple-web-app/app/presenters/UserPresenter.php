<?php

use Nette\Application\UI\Form;
use Nette\Security as NS;

class UserPresenter extends BasePresenter
{
    private $userRepository;

    private $authenticator;

    protected function startup()
    {
        parent::startup();
        $this->userRepository = $this->context->userRepository;
        $this->authenticator = $this->context->authenticator;
	if (!$this->getUser()->isLoggedIn()) {
	    $this->flashMessage('Pro spřístupnění toho obsahu se prosím přihlašte.', 'error');
	    $this->redirect('Homepage:');
	}
    }

    private function actionModifyuser() {
	if (!$this->getUser()->isInRole('admin')) {
	    $this->flashMessage('Pro vykonání této akce nemáte dostatečné práva', 'error');
	    $this->redirect('Homepage:');
	}
	
    }

    public function renderModifyuser() {
		//$this->template->users = $this->userRepository->findBy(array('role' => 'user'));
		$this->template->users = $this->userRepository->findAll();
    }

    protected function createComponentPasswordForm()
    {
        $form = new Form();
        $form->addPassword('oldPassword', 'Staré heslo:', 30)
            ->addRule(Form::FILLED, 'Je nutné zadat staré heslo.');
        $form->addPassword('newPassword', 'Nové heslo:', 30)
			->addRule(Form::FILLED, 'Je nutné zadat nové heslo.')
            ->addRule(Form::MIN_LENGTH, 'Nové heslo musí mít alespoň %d znaků.', 5);
        $form->addPassword('confirmPassword', 'Potvrzení hesla:', 30)
            ->addRule(Form::FILLED, 'Nové heslo je nutné zadat ještě jednou pro potvrzení.')
            ->addRule(Form::EQUAL, 'Zadná hesla se musejí shodovat.', $form['newPassword']);
        $form->addSubmit('set', 'Změnit heslo');
        $form->onSuccess[] = $this->passwordFormSubmitted;
        return $form;
    }


    public function passwordFormSubmitted(Form $form)
    {
        $values = $form->getValues();
        $user = $this->getUser();
        try {
            $this->authenticator->authenticate(array($user->getIdentity()->username, $values->oldPassword));
            $this->userRepository->setPassword($user->getId(), $values->newPassword);
            $this->flashMessage('Heslo bylo úspěšně změněno.', 'success');
            $this->redirect('this');
        } catch (NS\AuthenticationException $e) {
            $form->addError('Zadané heslo není správné.');
        }
    }
	
    private $record;
    public function actionEdituser($user_id) {
	    if (!$this->getUser()->isInRole('admin') && $user_id != $this->getUser()->getIdentity()->user_id) {
		    $this->flashMessage('Pro vykonání této akce nemáte dostatečná práva', 'error');
		    $this->redirect('Clanky:');
	    }

	    $this->record= $this->userRepository->findBy(array('user_id' => $user_id))->fetch();
	    if (!$this->record) {
		    $this->setView('notFound');
	    }
	    $this['editUserForm']->setDefaults(array(
		    'fullname' => $this->record['fullname'],
		    'username' => $this->record['username'],
		    'role' => $this->record['role']
	    ));
    }
	
	protected function createComponentEditUserForm()
    {
		$form = new Form;
		$form->addText('fullname', 'Vaše celé jméno:', 30);
		$form->addText('username', 'Uživatelské jmeno:', 30)
			->setRequired('Musíte zadat vaše uživatelské jméno.');
		$role = array(
			'admin' => 'admin',
			'user' => 'user'
		);
		$form->addRadioList('role', 'Role:', $role)
			->getSeparatorPrototype()->setName(NULL);
		// Nemuzu zmenit roli sam sobe
		if ($this->record['user_id'] == $this->getUser()->getIdentity()->user_id) {
			$form->getComponent('role')->setDisabled();
		}
		$form->addSubmit('edit', 'Upravit')
			->onClick[] = $this->editUserFormSubmitted;
		$form->addSubmit('cancel', 'Storno')
	    ->setValidationScope(FALSE)
	    ->onClick[] = callback($this, 'editUserFormCancel');
		return $form;
    }
	
    public function editUserFormSubmitted(Nette\Forms\Controls\SubmitButton $btn) {
	$values = $btn->form->getValues();
	if (!isset($values->role)) {
	    $values->role = $this->getUser()->getRoles();
	}
	$row = $this->userRepository->findByName($values->username);
	if ($row) { // uzivatel existuje, vyhodime vyjimku
		$btn->form->addError('Uživatel se zadaným jménem již existuje, zvolte prosím jiné uživatelské jméno');
		return;
	}
	
	try {
		$this->userRepository->getConnection()->exec(
		'UPDATE user SET username=?, fullname=?, role=? WHERE user_id=?',
		$values->username, $values->fullname, $values->role, $this->record->user_id);
		$this->flashMessage('Operace byla úspěšně provedena', 'success');
		$this->redirect('Homepage:');
	} catch (Exception $e) {
		if (!($e instanceof Nette\Application\AbortException)) {
			$this->flashMessage('Operaci nebylo možné provést', 'error');
		}
		$this->redirect('Homepage:');
	}
    }
	
	public function editUserFormCancel(Nette\Forms\Controls\SubmitButton $btn) {
		$this->redirect('Homepage:');
    }

    public function deleteUser($id) 
    {
	$this->userRepository->deleteUser($id);
	$this->flashMessage('Uživatel byl odstraněn', 'success');
	$this->redirect('User:modifyuser');
    }
	
}
