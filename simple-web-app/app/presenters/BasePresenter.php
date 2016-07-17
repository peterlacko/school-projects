<?php

use Nette\Application\UI\Form;
use Nette\Security;
include __DIR__ . '/../components/ConfirmationDialog/ConfirmationDialog.php';
/**
 * Base presenter for all application presenters.
 */
abstract class BasePresenter extends Nette\Application\UI\Presenter
{
    private $skolaRepository;
	private $citatyRepository;

    protected function startup()
    {
	parent::startup();
		$this->skolaRepository = $this->context->skolaRepository;
		$this->citatyRepository = $this->context->citatyRepository;
    }

    public function beforeRender()
    {
		$this->template->citat = $this->citatyRepository->getRandom();
		$this->template->skoly = $this->skolaRepository->findAll()->order('Nazev_skoly ASC');
    }

    /*
     * Sign-in form factory.
     * @return Nette\Application\UI\Form
     */
    protected function createComponentSignInForm()
    {
	    $form = new Form;
	    $form->setDefaults(array('username'=>'', 'password'=>''));
	    $form->addText('username', 'Uživatel:')
		->setRequired('Zadejte prosím jméno')
		->addRule(Form::MIN_LENGTH, 'Heslo musí mít alespoň %d znaky', 3);
	    $form->addPassword('password', 'Heslo:')
		->setRequired('Zadejte prosím heslo');
	    $form->addCheckbox('permanent', 'Trvalé přihlášení');
	    $form->addSubmit('login', 'Přihlásit');
	    
	    $form->setMethod('POST');
	    $form->onSuccess[] = $this->signInFormSubmitted;
	    return $form;
    }

    /*
     * prihlaseni uzivatele
     */
    public function signInFormSubmitted(Form $form)
    {
	try {
	    $user = $this->getUser();
	    $values = $form->getValues();
	    if ($values->permanent) {
		$user->setExpiration('+14 days', FALSE);
	}
	    $user->login($values->username, $values->password);
	    $this->presenter->flashMessage('Přihlášení bylo úspěšné.', 'success');
	    $this->redirect('this');
	} catch (Security\AuthenticationException $e) {
	    $this->flashMessage('Neplatné uživatelské jméno nebo heslo', 'error');
	}
    }

    /*
     * odhlaseni uzivatele
     */
    public function handleSignOut()
    {
	$this->getUser()->logout();
	$this->flashMessage('Byl jste úspěšně odhlášen', 'success');
	$this->redirect('Homepage:');
    }

	/* 
    /* 
     * metody pro potvrzovaci dialog
     */
    function createComponentConfirmForm()
    {
	$form = new ConfirmationDialog();
	$form->dialogClass = 'static_dialog second';
	$form->getFormButton('yes')->getControlPrototype()->addClass('yesbut');
	$form->getFormButton('no')->getControlPrototype()->addClass('nobut');

	$form ->addConfirmer(
	    'deleteSkola', // název signálu bude 'confirmDeleteSkola!'
	    array($this, 'deleteSkola'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Opravdu chcete smazat skolu "%s"?', $params['nazev']);
	    });
	$form ->addConfirmer(
	    'deleteFakulta', // název signálu bude 'confirmDeleteFakulta!'
	    array($this, 'deleteFakulta'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Opravdu chcete smazat fakultu "%s"?', $params['nazev']);
	    });
	$form ->addConfirmer(
	    'deleteObor', // název signálu bude 'confirmDeleteObor!'
	    array($this, 'deleteObor'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Opravdu chcete smazat obor "%s"?', $params['nazev']);
	    });
	$form ->addConfirmer(
	    'deleteDod', // název signálu bude 'confirmDeleteDod!'
	    array($this, 'deleteDod'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Opravdu chcete smazat tento Den otevřených dveří?');
	    });
	$form ->addConfirmer(
	    'deletePredmet', // název signálu bude 'confirmDeletePredmet!'
	    array($this, 'deletePredmet'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Opravdu chcete smazat tento předmět?');
	    });
	$form ->addConfirmer(
	    'deleteAkce', // název signálu bude 'confirmDeleteAkce!'
	    array($this, 'deleteAkce'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Opravdu chcete smazat tuto Akci?');
	    });
	$form ->addConfirmer(
	    'deleteClanek', // název signálu bude 'confirmDeleteClanek!'
	    array($this, 'deleteClanek'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Opravdu chcete smazat tento článek?');
	    });
	$form->addConfirmer(
	    'deleteOblibene', // název signálu bude 'confirmDeleteOblibene!'
	    array($this, 'deleteOblibene'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Odstranit položku z oblíbených?');
	    });
	$form->addConfirmer(
	    'deleteUser', // název signálu bude 'confirmDeleteUser!'
	    array($this, 'deleteUser'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Smazat uživatele %s?', $params['username']);
	    });
	$form->addConfirmer(
	    'deleteZamereni', // název signálu bude 'confirmDeleteZamereni!'
	    array($this, 'deleteZamereni'), // callback na funkci při kliku na YES
	    function ($dialog, $params) {
		return sprintf('Smazat zaměření %s?', $params['nazev_zamereni']);
	    });


    return $form;
    }

    public function createComponentQuickSearchForm()
    {
	$form = new Form;
	$form->addText('quick_search')
	    ->addRule(Form::MIN_LENGTH, 'Hledaný text musí obsahovat alespoň 3 znaky', 3);
	$form->addImage('search', "__DIR__/../../www/css/images/search.gif");
	$form->onSuccess[] = $this->quickSearchFormSubmitted;
	return $form;
    }

    public function quickSearchFormSubmitted(Form $form) 
    {
	$values = $form->getValues();
	$this->redirect('Search:results', array(
	    'phrase'=>$values->quick_search,
	    'skola'=>TRUE,
	    'fakulta'=>TRUE,
	    'obor'=>TRUE,
	    'akce'=>TRUE
	));
    }
}
